#include "ISO14443Analyzer.h"
#include "ISO14443AnalyzerSettings.h"
#include <AnalyzerChannelData.h>
#include <iomanip>
#include <sstream>

ISO14443Analyzer::ISO14443Analyzer()
:	Analyzer2(),  
	mSettings( new ISO14443AnalyzerSettings() ),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( mSettings.get() );
	output_file.open("C:\\Users\\Michael\\Desktop\\output.txt");
}

ISO14443Analyzer::~ISO14443Analyzer()
{
	output_file.close();
	KillThread();
}

void ISO14443Analyzer::SetupResults()
{
	mResults.reset(new ISO14443AnalyzerResults(this, mSettings.get()));
	SetAnalyzerResults(mResults.get());
	mResults->AddChannelBubblesWillAppearOn(mSettings->mInputChannel);

}

void ISO14443Analyzer::WorkerThread()
{

	
	mSampleRateHz = GetSampleRate();

	mSerial = GetAnalyzerChannelData(mSettings->mInputChannel);

	if (mSerial->GetBitState() == BIT_LOW)
		mSerial->AdvanceToNextEdge();

	
	
	double current_edge = mSerial->GetSampleNumber();
	double last_edge = 0;
	count_bitstream = 1;
	int start_bitstream;
	int count_edges = 0;
	int time_tolerance = 70;
	int bit_period = 945;
	bool isBitstream = false;
	mSerial->AdvanceToNextEdge();
	bit_stream_integer = 1;

	//convert to hex
	int bin[9];
	int bit_stream_length = 0;
	vector<int> bit_stream;
	

	for (; ; )
	{
		count_edges += 1;
		current_edge = mSerial->GetSampleNumber();


		if (isBitstream == false)
		{
			if (mSerial->GetSampleOfNextEdge() - current_edge < 400 && mSerial->GetSampleOfNextEdge() - current_edge > 300) {
				output_file << "Start of Bitstream at " << current_edge << "\n";
				mResults->AddMarker(current_edge, AnalyzerResults::Stop, mSettings->mInputChannel);
				start_bitstream = current_edge;
				isBitstream = true;
				

			}
			else
			{
				mSerial->AdvanceToNextEdge();
			}
		}


		if (isBitstream == true)
		{

			if (!mSerial->WouldAdvancingCauseTransition(1900))
			{
				
				PrintOutBitstream(bit_stream, bit_stream_length);
				UnpackBitstream(bit_stream, bit_stream_length);
				bit_stream_length = 0;
				bit_stream.resize(0);
				
				output_file << dec  << "end of bitstream " << count_bitstream << " at " << current_edge << "\n\n\n";
				
				
				Frame frame;		
				frame.mData1 = 0;
				frame.mFlags = 0;
				frame.mType = BITSTREAM;
				frame.mStartingSampleInclusive = start_bitstream;
				frame.mEndingSampleInclusive = current_edge;
				mResults->AddFrame(frame);
			
				count_bitstream += 1;
				isBitstream = false;
				bit_stream_integer = 0;
				mSerial->AdvanceToNextEdge();
			}

			if (isBitstream == true)
			{
				if (mSerial->WouldAdvancingCauseTransition(930))
				{
					if (mSerial->GetBitState() == BIT_HIGH)
					{
						// 1
						mResults->AddMarker(mSerial->GetSampleNumber() + 100, AnalyzerResults::One, mSettings->mInputChannel);
						bit_stream_length++;
						bit_stream.reserve(bit_stream_length-1);
						bit_stream.push_back(1);
					}

					if (mSerial->GetBitState() == BIT_LOW)
					{
						// 0a0
						mResults->AddMarker(mSerial->GetSampleNumber() + 100, AnalyzerResults::Zero, mSettings->mInputChannel);
						bit_stream_length++;
						bit_stream.reserve(bit_stream_length-1);
						bit_stream.push_back(0);						
					}
				}
				else
				{
					// 0a1
					mResults->AddMarker(mSerial->GetSampleNumber() + 100, AnalyzerResults::Zero, mSettings->mInputChannel);
					bit_stream_length++;
					bit_stream.reserve(bit_stream_length-1);
					bit_stream.push_back(0);
				}

				AdvanceToNextBit();
			}

			last_edge = current_edge;
		}


		mResults->CommitResults();
		ReportProgress(mSerial->GetSampleNumber());
	}
	
}

void ISO14443Analyzer::PrintOutBitstream(vector<int>& bit_stream, int bit_stream_length)
{
	
	for (int i = 0; i <= bit_stream_length - 1; i++)
	{
		
		output_file << bit_stream[i];

	}
	
	output_file << endl;
}

void ISO14443Analyzer::PrintOutDecodedBitstream(vector<int>& decoded_bit_stream)
{
	stringstream ss;
	
	for (int i = 0; i <= decoded_bit_stream.size() - 1; i++)
	{
		
		
		output_file << "      ";
	   
		output_file << hex << setw(2) << setfill('0') << uppercase << decoded_bit_stream[i];
		ss << hex << setw(2) << setfill('0') << uppercase << decoded_bit_stream[i];
		
		
		output_file << "   ";
		
	}
	std::string out_string = ss.str();
	output_string.push_back(out_string);

	output_file << " output_string:   ---- " << out_string;
	
		
}

void ISO14443Analyzer::AdvanceToNextBit()
{
	mSerial->Advance(900);
	if (mSerial->WouldAdvancingCauseTransition(47))
	{

		mSerial->AdvanceToNextEdge();
		mResults->AddMarker(mSerial->GetSampleNumber(), AnalyzerResults::UpArrow, mSettings->mInputChannel);
	}
	else {
		mSerial->Advance(45);
		mResults->AddMarker(mSerial->GetSampleNumber(), AnalyzerResults::UpArrow, mSettings->mInputChannel);
	}
}

void ISO14443Analyzer::UnpackBitstream(vector<int>& bit_stream, int bit_stream_length)
{
	vector<int> unpacked_bit_stream;

	// divided by 9 because we have a parity bit after each data byte
	unpacked_bit_stream.reserve(bit_stream_length/9);
	int byte = 0;
	// first bit is always 0 - so we start at 2nd position (index 1)
	int bit = 1;
	int data_bits;
	int parity_bits;

	if (bit_stream[bit_stream_length - 1] == 0)
	{
		bit_stream_length--;
	}
	if (bit_stream_length >= 10)
	{
		data_bits = 8;
		parity_bits = 1;
		
		output_file << endl << "SOF";
		for (int i = 1; i <= bit_stream_length / 9; i++)
		{
			output_file << "  DATA  |P|";
		}
		
		output_file << "EOF" << endl << "|" << bit_stream[0] << "|";
		
	}
	else
	{
		data_bits = 7;
		parity_bits = 0;
		
		output_file << "This is a short frame!" << endl << "SOF DATA  EOF" << endl;
		output_file << "|" << bit_stream[0] << "|";
		
	}
	
		while (bit_stream_length  - bit >= data_bits + parity_bits)
		{
			
			unpacked_bit_stream.push_back(0);
			
			for (int i = 0; i <= data_bits-1; i++)
			{
			
				output_file << bit_stream[bit];
				//Least significant bit is 
				unpacked_bit_stream[byte] += (bit_stream[bit])*pow(2, (i));
				bit++;
				
			}

			// skip the parity bit
			
			output_file << "|" << bit_stream[bit] <<"|" ;
			
			bit++;

			byte++;
		}
		
		if (bit_stream_length >= 10 && bit_stream[bit_stream_length-1]==0)
		{
			output_file << "|" << bit_stream[bit] << "|" << " bits left:" << bit_stream_length - bit << endl;
		}
		else if (bit_stream_length >=10){
			output_file << " bits left:" << bit_stream_length - bit << endl;
		}
		
			PrintOutDecodedBitstream(unpacked_bit_stream);
	
	}



bool ISO14443Analyzer::NeedsRerun()
{
	return false;
}

U32 ISO14443Analyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
		if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitilized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

string ISO14443Analyzer::GetResultString(U64 id)
{
	
	if (id > output_string.size() || id < 0)
	{
		output_file << "Error index out of bound:  " << id << " of [0-" << output_string.size()<< "]"<< " count_bitstream: " << count_bitstream<< endl;
		return "Error index out of bound"+id;
	}
	string out_str = output_string.at(id);
	const char* test = out_str.c_str();
	

	return out_str;
}

U32 ISO14443Analyzer::GetMinimumSampleRateHz()
{
	return mSettings->mBitRate * 4;
}

const char* ISO14443Analyzer::GetAnalyzerName() const
{
	return "NFC Sniffer";
}

const char* GetAnalyzerName()
{
	return "NFC Sniffer";
}

Analyzer* CreateAnalyzer()
{
	return new ISO14443Analyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}