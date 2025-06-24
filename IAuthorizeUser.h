#pragma once

#include <qstring.h>
#include <string>

class IAuthorizeUser
{
public:
	// returns user permission level 0=Operator, 10=ksiegowy, 20=userAdmin; -1=authentication failure
	virtual QString authorize_logIn(std::string username, std::string password) = 0;

	virtual void authorize_logOut() = 0;
};