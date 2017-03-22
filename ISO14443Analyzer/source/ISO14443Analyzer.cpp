#include "ISO14443Analyzer.h"
#include "ISO14443AnalyzerSettings.h"
#include <AnalyzerChannelData.h>

ISO14443Analyzer::ISO14443Analyzer()
:	Analyzer(),  
	mSettings( new ISO14443AnalyzerSettings() ),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( mSettings.get() );
	myfile.open("C:\\Users\\Michael\\Desktop\\output.txt");
}

ISO14443Analyzer::~ISO14443Analyzer()
{
	myfile.close();
	KillThread();
}

void ISO14443Analyzer::binToHex(int* bin)
{
	
	char hex[2];
	int dec = 0;

	for (int i = 7; i >= 0; i--)
	{
		dec += (bin[i])*pow(2, (7 - i));
	}
	int second = dec % 16;
	int first = dec / 16;
	hex[0] = convertDecToHex(first);
	hex[1] = convertDecToHex(second);
	myfile << " --- " << hex[0] << hex[1] << "\n";
}

char ISO14443Analyzer::convertDecToHex(int dec)
{
	if (dec<10)
	{
		return '0' + dec;
	}
	return 'A' + dec - 10;
}

void ISO14443Analyzer::WorkerThread()
{

	mResults.reset(new ISO14443AnalyzerResults(this, mSettings.get()));
	SetAnalyzerResults(mResults.get());
	mResults->AddChannelBubblesWillAppearOn(mSettings->mInputChannel);
	mSampleRateHz = GetSampleRate();

	mSerial = GetAnalyzerChannelData(mSettings->mInputChannel);

	if (mSerial->GetBitState() == BIT_LOW)
		mSerial->AdvanceToNextEdge();

	double current_edge = mSerial->GetSampleNumber();
	double last_edge = 0;
	int count_bitstream = 1;
	int count_edges = 0;
	int time_tolerance = 70;
	int bit_period = 945;
	bool isBitstream = false;
	mSerial->AdvanceToNextEdge();

	//convert to hex
	int bin[9];
	int countpos = 0;
	

	for (; ; )
	{
		count_edges += 1;
		current_edge = mSerial->GetSampleNumber();


		if (isBitstream == false)
		{
			if (mSerial->GetSampleOfNextEdge() - current_edge < 400 && mSerial->GetSampleOfNextEdge() - current_edge > 300) {
				myfile << "Start of Bitstream at " << current_edge << "\n";
				mResults->AddMarker(current_edge, AnalyzerResults::Stop, mSettings->mInputChannel);
				isBitstream = true;
				AdvanceToNextBit();

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
				if (countpos == 8)
				{
					for (int i = 8; i>=1; i--)
					{	
					  bin[i] = bin[i - 1];
					  myfile << bin[0] << bin[1] << bin[2] << bin[3] << bin[4] << bin[5] << bin[6] << bin[7] << bin[8] ;
					}
					bin[0] = 0;
					myfile << bin[0] << bin[1] << bin[2] << bin[3] << bin[4] << bin[5] << bin[6] << bin[7] <<;
					binToHex(bin);
					myfile << "\n" << "this is a short bitstream with " << countpos << "\n";
					countpos = 0;
				}

				myfile << "end of bitstream " << count_bitstream << " at " << current_edge << "\n\n\n";
				count_bitstream += 1;
				isBitstream = false;
				
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
						bin[countpos] = 1;
						countpos = countpos+1;
						if (countpos == 9)
						{
							myfile <<bin[0] << bin[1] << bin[2] << bin[3] << bin[4] << bin[5] << bin[6] << bin[7] << " parity bit: " << bin[8];
							binToHex(bin);
							countpos = 0;

						}
					}

					if (mSerial->GetBitState() == BIT_LOW)
					{
						// 0a0
						mResults->AddMarker(mSerial->GetSampleNumber() + 100, AnalyzerResults::Zero, mSettings->mInputChannel);
						bin[countpos] = 0;
						countpos = countpos + 1;
						if (countpos == 9)
						{
							myfile << bin[0] << bin[1] << bin[2] << bin[3] << bin[4] << bin[5] << bin[6] << bin[7] << " parity bit: " << bin[8];
							binToHex(bin);
							countpos = 0;

						}
					}
				}
				else
				{
					// 0a1
					mResults->AddMarker(mSerial->GetSampleNumber() + 100, AnalyzerResults::Zero, mSettings->mInputChannel);
					bin[countpos] = 0;
					countpos = countpos + 1;
					if (countpos == 9)
					{
						myfile << bin[0] << bin[1] << bin[2] << bin[3] << bin[4] << bin[5] << bin[6] << bin[7] << " parity bit: " << bin[8] ;
						binToHex(bin);
						countpos = 0;
					}
				}

				AdvanceToNextBit();
			}

			last_edge = current_edge;
		}


		mResults->CommitResults();
	}
	
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