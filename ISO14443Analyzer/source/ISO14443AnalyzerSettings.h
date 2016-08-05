#ifndef ISO14443_ANALYZER_SETTINGS
#define ISO14443_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class ISO14443AnalyzerSettings : public AnalyzerSettings
{
public:
	ISO14443AnalyzerSettings();
	virtual ~ISO14443AnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	
	Channel mInputChannel;
	U32 mBitRate;

protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mInputChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mBitRateInterface;
};

#endif //ISO14443_ANALYZER_SETTINGS
