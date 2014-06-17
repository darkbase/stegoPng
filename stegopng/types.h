#pragma once

typedef std::vector<char> BinaryVector;

// ref
typedef const BinaryVector& BinaryVector_p;

// ptr
typedef std::tr1::shared_ptr<BinaryVector> BinaryVector_ptr;
typedef const BinaryVector_ptr& BinaryVector_ptr_p;

// map
typedef std::map<int, BinaryVector_ptr> BinaryVectorMap;
typedef std::tr1::shared_ptr<BinaryVectorMap> BinaryVectorMap_ptr;
typedef const BinaryVectorMap_ptr& BinaryVectorMap_ptr_p;

typedef std::vector<int> IntVector;

// ref
typedef const IntVector& IntVector_p;

// ptr
typedef std::tr1::shared_ptr<IntVector> IntVector_ptr;
typedef const IntVector_ptr& IntVector_ptr_p;

// list<string>
typedef std::list<std::string> StringList;
typedef const StringList& StringList_p;
typedef std::tr1::shared_ptr<StringList> StringList_ptr;
typedef const StringList_ptr& StringList_ptr_p;

// string
typedef std::tr1::shared_ptr<std::string> string_ptr;

//list<BinaryVector>
typedef std::list<BinaryVector_ptr> BinaryVectorList;
typedef std::tr1::shared_ptr<BinaryVectorList> BinaryVectorList_ptr;
typedef const BinaryVectorList_ptr& BinaryVectorList_ptr_p;


struct classcomp {
	template<typename T>
	bool operator()(const std::tr1::shared_ptr<T>& lhs, const std::tr1::shared_ptr<T>& rhs)
	{
		return *lhs < *rhs;
	}
};