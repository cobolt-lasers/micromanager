/**
 * \file        Laser.cpp
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#include <assert.h>
#include "Laser.h"
#include "Property.h"

#define ArrayEnd( arr ) (arr + sizeof( arr ) / sizeof( arr[ 0 ] ) )

using namespace std;
using namespace cobolt;

const char* Laser::Milliamperes = "mA";
const char* Laser::Amperes = "A";
const char* Laser::Milliwatts = "mW";
const char* Laser::Watts = "W";

Laser* Laser::Create( LaserDevice* device )
{
    assert( device != NULL );
    
    std::string modelString;
    if ( device->SendCommand( "glm?", &modelString ) != return_code::ok ) {
        return NULL;
    }
    
    std::vector<std::string> modelTokens;
    DecomposeModelString( modelString, modelTokens );
    Laser* laser;
    
    if ( modelString.find( "-06-" ) != std::string::npos ) {

        laser = new Laser( "06-DPL", device );

        StringValueMap runModes[] = {
            { "0", "Constant Current" },
            { "1", "Constant Power" },
            { "2", "Modulation" }
        };

        laser->currentUnit_ = Milliamperes; 
        laser->powerUnit_ = Milliwatts;
        laser->maxCurrentSetpoint_ = 3.0f;
        laser->maxPowerSetpoint_ = 100.0f; // TODO: Pick proper value.

        laser->CreateNameProperty();
        laser->CreateModelProperty();
        laser->CreateWavelengthProperty();
        laser->CreateSerialNumberProperty();
        laser->CreateFirmwareVersionProperty();
        laser->CreateOperatingHoursProperty();
        laser->CreateCurrentSetpointProperty();
        laser->CreateCurrentReadingProperty();
        laser->CreatePowerSetpointProperty();
        laser->CreatePowerReadingProperty();
        laser->CreateToggleProperty();
        laser->CreatePausedProperty();
        laser->CreateRunModeProperty( std::vector<StringValueMap>( runModes, ArrayEnd( runModes ) ) );
        laser->CreateDigitalModulationProperty();
        laser->CreateAnalogModulationFlagProperty();
        laser->CreateModulationPowerSetpointProperty();
        laser->CreateAnalogImpedanceProperty();

    } else if ( modelString.find( "-05-" ) != std::string::npos ) {

        laser = new Laser( "Compact 05", device );

        StringValueMap runModes[] = {
            { "0", "Constant Current" },
            { "1", "Constant Power" }
        };

        laser->currentUnit_ = Amperes;
        laser->powerUnit_ = Milliwatts;
        laser->maxCurrentSetpoint_ = 3.0f;
        laser->maxPowerSetpoint_ = 100.0f; // TODO: Pick proper value.

        laser->CreateNameProperty();
        laser->CreateModelProperty();
        laser->CreateWavelengthProperty();
        laser->CreateSerialNumberProperty();
        laser->CreateFirmwareVersionProperty();
        laser->CreateOperatingHoursProperty();
        laser->CreateCurrentSetpointProperty();
        laser->CreateCurrentReadingProperty();
        laser->CreatePowerSetpointProperty();
        laser->CreatePowerReadingProperty();
        laser->CreateToggleProperty();
        laser->CreatePausedProperty();
        laser->CreateRunModeProperty( std::vector<StringValueMap>( runModes, ArrayEnd( runModes ) ) );
        
    } else {

        laser = new Laser( "Unknown", device );
    }
    
    if ( modelTokens.size() > 0 ) {
        laser->wavelength_ = modelTokens[ 0 ];
    } else {
        laser->wavelength_ = "Unknown";
    }
    
    Logger::Instance()->LogMessage( "Created laser '" + laser->GetName() + "'", true );

    return laser;
}

Laser::~Laser()
{
    const bool pausedPropertyIsPublic = ( pausedProperty_ != NULL && properties_.find( pausedProperty_->GetName() ) != properties_.end() );
    
    if ( !pausedPropertyIsPublic ) {
        delete pausedProperty_;
    }

    for ( PropertyIterator it = GetPropertyIteratorBegin(); it != GetPropertyIteratorEnd(); it++ ) {
        delete it->second;
    }

    properties_.clear();
}

const std::string& Laser::GetName() const
{
    return name_;
}

const std::string& Laser::GetWavelength() const
{
    return wavelength_;
}

void Laser::SetOn( const bool on )
{
    toggleProperty_->Set( ( on ? value::toggle::on.guiValue : value::toggle::off.guiValue ) );
}

void Laser::SetPaused( const bool paused )
{
    pausedProperty_->Set( ( paused ? value::toggle::on.guiValue : value::toggle::off.guiValue ) );
}

bool Laser::IsOn() const
{
    return ( toggleProperty_->Get<std::string>() == value::toggle::on.guiValue );
}

bool Laser::IsPaused() const
{
    return ( pausedProperty_->Get<std::string>() == value::toggle::on.guiValue );
}

Property* Laser::GetProperty( const std::string& name ) const
{
    return properties_.at( name );
}

Property* Laser::GetProperty( const std::string& name )
{
    return properties_[ name ];
}

Laser::PropertyIterator Laser::GetPropertyIteratorBegin()
{
    return properties_.begin();
}

Laser::PropertyIterator Laser::GetPropertyIteratorEnd()
{
    return properties_.end();
}

/**
 * \brief Extracts the string parts from the glm? command and put them one by one in a vector.
 *        expects a string where the parts are separated with the character '-'
 *
 * \example The model string could have a format similar to 'WWWW-06-XX-PPPP-CCC'.
 */
void Laser::DecomposeModelString( std::string modelString, std::vector<std::string>& modelTokens )
{
    std::string token;

    for ( std::string::iterator its = modelString.begin(); its != modelString.end(); its++ ) {

        if ( *its != '-' ) {

            token.push_back( *its );

        } else if ( *its == '\r' ) {

            modelTokens.push_back( token );
            break;

        } else {

            modelTokens.push_back( token );
            token.clear();
        }
    }
}

Laser::Laser( const std::string& name, LaserDevice* device ) :
    name_( name ),
    wavelength_( "Unknown" ),
    device_( device ),
    maxCurrentSetpoint_( 0.0f ),
    maxPowerSetpoint_( 0.0f ),
    currentUnit_( "?" ),
    powerUnit_( "?" )
{
    CreateNameProperty();
    CreateModelProperty();
    CreateWavelengthProperty();
    CreateSerialNumberProperty();
    CreateFirmwareVersionProperty();
    CreateOperatingHoursProperty();
    CreatePausedProperty();
    CreateToggleProperty();
}

void Laser::CreateNameProperty()
{
    RegisterPublicProperty( new StaticStringProperty( "Name", this->GetName() ) );
}

void Laser::CreateModelProperty()
{
    RegisterPublicProperty( new BasicProperty<std::string>( "Model", device_, "glm?") );
}

void Laser::CreateWavelengthProperty()
{
    RegisterPublicProperty( new StaticStringProperty( "Wavelength", this->GetWavelength()) );
}

void Laser::CreateSerialNumberProperty()
{
    RegisterPublicProperty( new BasicProperty<std::string>( "Serial Number", device_, "gsn?") );
}

void Laser::CreateFirmwareVersionProperty()
{
    RegisterPublicProperty( new BasicProperty<std::string>( "Firmware Version", device_, "gfv?") );
}

void Laser::CreateOperatingHoursProperty()
{
    RegisterPublicProperty( new BasicProperty<std::string>( "Operating Hours", device_, "hrs?") );
}

void Laser::CreateCurrentSetpointProperty()
{
    MutableProperty* property = new NumericProperty<double>( "Current Setpoint [" + currentUnit_ + "]", device_, "glc?", "slc", 0.0f, maxCurrentSetpoint_ );
    RegisterPublicProperty( property );
}

void Laser::CreateCurrentReadingProperty()
{
    RegisterPublicProperty( new BasicProperty<double>( "Measured Current [" + currentUnit_ + "]", device_, "i?") );
}

void Laser::CreatePowerSetpointProperty()
{
    MutableProperty* property = new NumericProperty<double>( "Power Setpoint [" + powerUnit_ + "]", device_, "glp?", "slp", 0.0f, maxPowerSetpoint_ );
    RegisterPublicProperty( property );
}

void Laser::CreatePowerReadingProperty()
{
    RegisterPublicProperty( new BasicProperty<double>( "Power Reading [" + powerUnit_ + "]", device_, "pa?") );
}

void Laser::CreateToggleProperty()
{
    toggleProperty_ = new BoolProperty( "On-Off Switch", device_, BoolProperty::OnOff, "l?", "l1", "l0" );
    RegisterPublicProperty( toggleProperty_ );
}

void Laser::CreatePausedProperty()
{
    if ( IsPauseCommandSupported() ) {
        pausedProperty_ = new LaserPausedProperty( "Paused", device_ );
    } else {
        pausedProperty_ = new LaserSimulatedPausedProperty( "Paused", device_ );
    }
    
    RegisterPublicProperty( pausedProperty_ ); // TODO: Consider removing as this property should not be public?
}

void Laser::CreateRunModeProperty( const std::vector<StringValueMap>& supportedRunModes )
{
    EnumerationProperty* property = new EnumerationProperty( "Run Mode", device_, "gam?", "sam" );

    for ( std::vector<StringValueMap>::const_iterator supportedRunMode = supportedRunModes.begin();
        supportedRunMode != supportedRunModes.end(); supportedRunMode++ ) {
        property->RegisterValidValue( *supportedRunMode );
    }

    RegisterPublicProperty( property );
}

void Laser::CreateDigitalModulationProperty()
{
    MutableProperty* property = new BoolProperty( "Digital Modulation", device_, BoolProperty::EnableDisable, "gdmes?", "sdmes 1", "sdmes 0" );
    RegisterPublicProperty( property );
}

void Laser::CreateAnalogModulationFlagProperty()
{
    MutableProperty* property = new BoolProperty( "Analog Modulation", device_, BoolProperty::EnableDisable, "games?", "sames 1", "sames 0" );
    RegisterPublicProperty( property );
}

void Laser::CreateModulationPowerSetpointProperty()
{
    RegisterPublicProperty( new BasicMutableProperty<double>( "Modulation Power Setpoint", device_, "glmp?", "slmp") );
}

void Laser::CreateAnalogImpedanceProperty()
{
    EnumerationProperty* property = new EnumerationProperty( "Analog Impedance", device_, "galis?", "salis" );
    
    property->RegisterValidValue( value::analog_impedance::low );
    property->RegisterValidValue( value::analog_impedance::high );

    RegisterPublicProperty( property );
}

bool Laser::IsPauseCommandSupported()
{
    std::string response;
    device_->SendCommand( "l0r", &response );
    
    return ( response.find( "OK" ) != std::string::npos );
}

void Laser::RegisterPublicProperty( Property* property )
{
    assert( property != NULL );
    properties_[ property->GetName() ] = property;
}
