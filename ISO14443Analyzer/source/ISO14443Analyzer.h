#ifndef ISO14443_ANALYZER_H
#define ISO14443_ANALYZER_H

#include <Analyzer.h>
#include "ISO14443AnalyzerResults.h"
#include "ISO14443SimulationDataGenerator.h"

class ISO14443AnalyzerSettings;
class ANALYZER_EXPORT ISO14443Analyzer : public Analyzer
{
private:
	char convertDecToHex(int dec);
	void binToHex(int* bin);
public:
	ISO14443Analyzer();
	virtual ~ISO14443Analyzer();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

protected: //vars
	std::auto_ptr< ISO14443AnalyzerSettings > mSettings;
	std::auto_ptr< ISO14443AnalyzerResults > mResults;
	AnalyzerChannelData* mSerial;

	ISO14443SimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitilized;

	//Serial analysis vars:
	U32 mSampleRateHz;
	U32 mStartOfStopBitOffset;
	U32 mEndOfStopBitOffset;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //ISO14443_ANALYZER_H
