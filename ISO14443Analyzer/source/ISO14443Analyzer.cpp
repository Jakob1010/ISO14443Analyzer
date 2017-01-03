#include "ISO14443Analyzer.h"
#include "ISO14443AnalyzerSettings.h"
#include <AnalyzerChannelData.h>


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

	U32 samples_per_bit = mSampleRateHz / mSettings->mBitRate;
	U32 samples_to_first_center_of_first_data_bit = U32(1.5 * double(mSampleRateHz) / double(mSettings->mBitRate));

	for (; ; )
	{
		U8 data = 0;
		U8 mask = 1 << 7;

		mSerial->AdvanceToNextEdge(); //falling edge -- beginning of the start bit

		U64 starting_sample = mSerial->GetSampleNumber();

		mSerial->Advance(samples_to_first_center_of_first_data_bit);
		mResults->AddMarker(3, AnalyzerResults::One, mSettings->mInputChannel);
		mResults->AddMarker(4, AnalyzerResults::Square, mSettings->mInputChannel); 
		mResults->AddMarker(5, AnalyzerResults::Stop, mSettings->mInputChannel);
		for (U32 i = 0; i<8; i++)
		{
			//let's put a dot exactly where we sample this bit:
			mResults->AddMarker(mSerial->GetSampleNumber(), AnalyzerResults::Dot, mSettings->mInputChannel);

			if (mSerial->GetBitState() == BIT_HIGH)
				data |= mask;

			mSerial->Advance(samples_per_bit);

			mask = mask >> 1;
		}


		//we have a byte to save. 
		Frame frame;
		frame.mData1 = data;
		frame.mFlags = 0;
		frame.mStartingSampleInclusive = starting_sample;
		frame.mEndingSampleInclusive = mSerial->GetSampleNumber();

		mResults->AddFrame(frame);
		mResults->CommitResults();
		ReportProgress(frame.mEndingSampleInclusive);
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