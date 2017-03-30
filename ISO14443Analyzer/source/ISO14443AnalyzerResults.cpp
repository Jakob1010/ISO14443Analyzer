#include "ISO14443AnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "ISO14443Analyzer.h"
#include "ISO14443AnalyzerSettings.h"
#include <iostream>
#include <fstream>


ISO14443AnalyzerResults::ISO14443AnalyzerResults(ISO14443Analyzer* analyzer, ISO14443AnalyzerSettings* settings)
	: AnalyzerResults(),
	mSettings(settings),
	mAnalyzer(analyzer)
	
{
	test1 = 1;
}

ISO14443AnalyzerResults::~ISO14443AnalyzerResults()
{
}

void ISO14443AnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base )
{
	ClearResultStrings();
	Frame frame = GetFrame( frame_index );
	const char* cstr = mAnalyzer->GetResultString();
	ofstream file_stream("C:\\Users\\Michael\\Desktop\\outputResults.txt");
	file_stream << "   .-. " << mAnalyzer->GetResultString() << cstr;
	//AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
	test1++;
	
	
	AddResultString(cstr);
}

void ISO14443AnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
	std::ofstream file_stream( file, std::ios::out );

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();

	file_stream << "Time [s],Value" << std::endl;

	U64 num_frames = GetNumFrames();
	for( U32 i=0; i < num_frames; i++ )
	{
		Frame frame = GetFrame( i );
		
		char time_str[128];
		AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );

		char number_str[128];
		AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );

		file_stream << time_str << "," << number_str << std::endl;

		if( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
		{
			file_stream.close();
			return;
		}
	}

	file_stream.close();
}

void ISO14443AnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
	Frame frame = GetFrame( frame_index );
	ClearResultStrings();

	char number_str[128];
	//AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
	AddResultString("homo");
}

void ISO14443AnalyzerResults::GeneratePacketTabularText( U64 packet_id, DisplayBase display_base )
{
	ClearResultStrings();
	AddResultString( "not supported" );
}

void ISO14443AnalyzerResults::GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base )
{
	ClearResultStrings();
	AddResultString( "not supported" );
}