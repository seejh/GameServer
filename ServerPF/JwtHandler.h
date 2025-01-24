#pragma once


/*---------------------------------------------------------------
	디코드
	string token = "~~.~~~.~~~";
	auto decoded = jwt::decode(token);
	for (auto& e : decoded.get_payload_json())
		cout << e.first << ":" << e.second << endl;

	토큰 검증
	auto verifier = jwt::verify()
	.with_issuer("")
	.with_claim("", jwt::claim(string("")))
	.allow_algorithm(jwt::algorithm::hs256{"secret"});

	verifier.verify(decoded_token);

	토큰 생성
	auto token = jwt::create()
	.set_type("JWS")
	.set_issuer("")
	.set_payload_cliam("", jwt::claim(string("")))
	.sign(jwt::algorithm::hs256 {"secret"});
---------------------------------------------------------------*/

class JwtHandler
{
public:
	// 토큰 검증, 결과 리턴
	static bool VerifyToken(string token, int32& accountDbId);
};

