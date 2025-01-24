#include "pch.h"
#include "JwtHandler.h"

#undef max
#undef min
#include"jwt-cpp/jwt.h"



bool JwtHandler::VerifyToken(string token, int32& accountDbId)
{
	// ���ڵ�
	auto decoded = jwt::decode(token);
	
	// ��ũ��Ű
	string secretKey("This is my Secret Secret Key for authenticatation");

	// Ȯ��
	auto verifier = jwt::verify().allow_algorithm(jwt::algorithm::hs256(secretKey));
	for (auto& e : decoded.get_payload_json()) {
		verifier.with_claim(e.first, jwt::claim(e.second));
		if (e.first.compare("UserName") == 0)
			accountDbId = stoi(e.second.to_str());
	}
	
	// ����
	try {
		verifier.verify(decoded);
	}
	catch (exception e) {
		cout << "[JwtHandler] VerifyToken Failed" << endl;
		return false;
	}

	cout << "[JwtHandler] VerifyToken Succeed" << endl;

	return true;
}

