#include "UserClass.h"

UserClass::UserClass()
{
	userType = "Aministrator";
}

UserClass::UserClass(QString permission_type) :
	userType(permission_type)
{
}

UserClass::~UserClass()
{
}
