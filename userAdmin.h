#pragma once
#include "UserClass.h"

#include "IUserManagement.h"

class userAdmin : public UserClass, public IUserManagement
{
public:
	userAdmin();
	~userAdmin();
};
