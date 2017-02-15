#include "ISO14443Analyzer.h"
#include "ISO14443AnalyzerSettings.h"
#include <AnalyzerChannelData.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

ISO14443Analyzer::ISO14443Analyzer()
:	Analyzer(),  
	mSettings( new ISO14443AnalyzerSettings() ),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( mSettings.get() );
}



ISO14443Analyzer::~ISO14443Analyzer()
{
	KillThread();
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

	ofstream myfile;
	myfile.open("C:\\Users\\Michael\\Desktop\\example.txt");
	
	double current_edge = mSerial->GetSampleNumber();
	double last_edge = 0;
	int count_bitstream = 1;
	int count_edges = 0;
	int time_tolerance = 70;
	int bit_period = 945;
	bool isBitstream = false;
	
	mSerial->AdvanceToNextEdge();

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
				}
				else
				{
					mSerial->AdvanceToNextEdge();
				}
			}
		

				if(isBitstream == true)
				{

				if (!mSerial->WouldAdvancingCauseTransition(1900))
				{
					mResults->AddMarker(current_edge, AnalyzerResults::Stop, mSettings->mInputChannel);
					myfile <<"end of bitstream " << count_bitstream << " at "<< current_edge <<"\n";
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
							myfile << "+++ detected a 1 +++ " << mSerial->GetSampleNumber() << "\n";
							mResults->AddMarker(mSerial->GetSampleNumber()+100, AnalyzerResults::One, mSettings->mInputChannel);
							
						}

						if (mSerial->GetBitState() == BIT_LOW)
						{
							myfile << "+++ detected a 0a0 at " << mSerial->GetSampleNumber() << " +++ \n";
							mResults->AddMarker(mSerial->GetSampleNumber()+100, AnalyzerResults::Zero, mSettings->mInputChannel);
						}
					}
					else
					{
						myfile << "+++ detected a 0a1 +++ " << mSerial->GetSampleNumber() << "\n";
						mResults->AddMarker(mSerial->GetSampleNumber()+100, AnalyzerResults::Zero, mSettings->mInputChannel);
					}

					mSerial->Advance(900);
					if (mSerial->WouldAdvancingCauseTransition(47))
					{
						
						mSerial->AdvanceToNextEdge();
						mResults->AddMarker(mSerial->GetSampleNumber(), AnalyzerResults::UpArrow, mSettings->mInputChannel);
					}
					else {
						mSerial->Advance(45);
						mResults->AddMarker(mSerial->GetSampleNumber(),AnalyzerResults::UpArrow, mSettings->mInputChannel);
					}
				}
				



				last_edge = current_edge;
				}
			
		
		mResults->CommitResults();
	}
	myfile.close();
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