/**
 * \file        LaserShutterProperty.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT__LASER_SHUTTER_PROPERTY_H
#define __COBOLT__LASER_SHUTTER_PROPERTY_H

#include "EnumerationProperty.h"

NAMESPACE_COBOLT_BEGIN

class Laser;

class LaserShutterProperty : public EnumerationProperty
{
public:

    static const std::string Value_Open;
    static const std::string Value_Closed;

    LaserShutterProperty( const std::string& name, LaserDriver* laserDriver, Laser* laser );
    
    virtual int GetValue( std::string& string ) const;
    virtual int SetValue( const std::string& );

private:

    Laser* laser_;
    bool isOpen_;
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__LASER_SHUTTER_PROPERTY_H