#include "pch.h"
#include "JwtHandler.h"

#undef max
#undef min
#include"jwt-cpp/jwt.h"



bool JwtHandler::VerifyToken(string token)
{
	auto decoded = jwt::decode(token);

	string secretKey("This is my custom Secret Key for authenticatation");

	auto verifier = jwt::verify().allow_algorithm(jwt::algorithm::hs256(secretKey));
	for (auto& e : decoded.get_payload_json())
		verifier.with_claim(e.first, jwt::claim(e.second));

	try {
		verifier.verify(decoded);
	}
	catch (exception e) {
		// TODO : Log
		return false;
	}

	return true;
}

// TODO :  토큰을 새로 만듬으로써 비교할려고
bool JwtHandler::MakeCompare(string token)
{
	string secretKey("This is my custom Secret Key for authenticatation");

	//auto token2 = jwt::create().

	return false;
}
