#pragma once
#include "Types.h"

NAMESPACE_BEGIN(DBModel)

USING_SHARED_PTR(Column);
USING_SHARED_PTR(Index);
USING_SHARED_PTR(Table);
USING_SHARED_PTR(Procedure);

/*-------------
	DataType
--------------*/

enum class DataType
{
	None = 0,
	TinyInt = 48,
	SmallInt = 52,
	Int = 56,
	Real = 59,
	DateTime = 61,
	Float = 62,
	Bit = 104,
	Numeric = 108,
	BigInt = 127,
	VarBinary = 165,
	Varchar = 167,
	Binary = 173,
	NVarChar = 231,
};

/*-------------
	Column
--------------*/

class Column
{
public:
	WString				CreateText();

public:
	WString				_name;
	int32				_columnId = 0; // DB
	DataType			_type = DataType::None;
	WString				_typeText;
	int32				_maxLength = 0;
	bool				_nullable = false;
	bool				_identity = false;
	int64				_seedValue = 0;
	int64				_incrementValue = 0;
	WString				_default;
	WString				_defaultConstraintName; // DB
};

/*-----------
	Index
------------*/

enum class IndexType
{
	Clustered = 1,
	NonClustered = 2
};

class Index
{
public:
	WString				GetUniqueName();
	WString				CreateName(const WString& tableName);
	WString				GetTypeText();
	WString				GetKeyText();
	WString				CreateColumnsText();
	bool				DependsOn(const WString& columnName);

public:
	WString				_name; // DB
	int32				_indexId = 0; // DB
	IndexType			_type = IndexType::NonClustered;
	bool				_primaryKey = false;
	bool				_uniqueConstraint = false;
	Vector<ColumnRef>	_columns;
};

/*-----------
	Table
------------*/

class Table
{
public:
	ColumnRef			FindColumn(const WString& columnName);

public:
	int32				_objectId = 0; // DB
	WString				_name;
	Vector<ColumnRef>	_columns;
	Vector<IndexRef>	_indexes;
};

/*----------------
	Procedures
-----------------*/

struct Param
{
	WString				_name;
	WString				_type;
};

class Procedure
{
public:
	WString				GenerateCreateQuery();
	WString				GenerateAlterQuery();
	WString				GenerateParamString();

public:
	WString				_name;
	WString				_fullBody; // DB
	WString				_body; // XML
	Vector<Param>		_parameters;  // XML
};

/*-------------
	Helpers
--------------*/

class Helpers
{
public:
	static WString		Format(const WCHAR* format, ...);
	static WString		DataType2String(DataType type);
	static WString		RemoveWhiteSpace(const WString& str);
	static DataType		String2DataType(const WCHAR* str, OUT int32& maxLen);
};

NAMESPACE_END