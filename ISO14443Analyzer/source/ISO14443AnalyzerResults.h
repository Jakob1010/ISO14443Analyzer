#ifndef ISO14443_ANALYZER_RESULTS
#define ISO14443_ANALYZER_RESULTS
#include <string>
#include <AnalyzerResults.h>
#include <fstream>
#include <iostream>

class ISO14443Analyzer;
class ISO14443AnalyzerSettings;

class ISO14443AnalyzerResults : public AnalyzerResults
{
private:
	int test1;
	std::ofstream file_stream;

public:
	ISO14443AnalyzerResults( ISO14443Analyzer* analyzer, ISO14443AnalyzerSettings* settings );
	virtual ~ISO14443AnalyzerResults();

	virtual void GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base);
	
	virtual void GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id );

	virtual void GenerateFrameTabularText(U64 frame_index, DisplayBase display_base );
	virtual void GeneratePacketTabularText( U64 packet_id, DisplayBase display_base );
	virtual void GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base );

protected: //functions

protected:  //vars
	ISO14443AnalyzerSettings* mSettings;
	ISO14443Analyzer* mAnalyzer;
};

#endif //ISO14443_ANALYZER_RESULTS
