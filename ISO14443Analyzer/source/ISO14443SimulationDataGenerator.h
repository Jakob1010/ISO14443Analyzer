#ifndef ISO14443_SIMULATION_DATA_GENERATOR
#define ISO14443_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>
#include <string>
class ISO14443AnalyzerSettings;

class ISO14443SimulationDataGenerator
{
public:
	ISO14443SimulationDataGenerator();
	~ISO14443SimulationDataGenerator();

	void Initialize( U32 simulation_sample_rate, ISO14443AnalyzerSettings* settings );
	U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel );

protected:
	ISO14443AnalyzerSettings* mSettings;
	U32 mSimulationSampleRateHz;

protected:
	void CreateSerialByte();
	std::string mSerialText;
	U32 mStringIndex;

	SimulationChannelDescriptor mSerialSimulationData;

};
#endif //ISO14443_SIMULATION_DATA_GENERATOR