/*
 * Copyright 2010-2015 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef OPENXCOM_SCRIPT_H
#define	OPENXCOM_SCRIPT_H

#include <map>
#include <vector>
#include <string>
#include <SDL_stdinc.h>


namespace OpenXcom
{
class Surface;
class ScriptContainerBase;

class ParserHelper;
struct SelectedToken;

template<typename T>
struct ArgSelector;

constexpr int ScriptMaxArg = 8;
constexpr int ScriptMaxReg = 256;

/**
 * Script execution cunter
 */
enum class ProgPos : size_t
{
	Unknown = (size_t)-1,
	Start = 0,
};

inline ProgPos& operator+=(ProgPos& pos, int offset)
{
	pos = static_cast<ProgPos>(static_cast<size_t>(pos) + offset);
	return pos;
}
inline ProgPos& operator++(ProgPos& pos)
{
	pos += 1;
	return pos;
}
inline ProgPos operator++(ProgPos& pos, int)
{
	ProgPos old = pos;
	++pos;
	return old;
}

/**
 * Args types
 */
enum ArgEnum : Uint8
{
	ArgNone,
	ArgProg,
	ArgContext,

	ArgReg,
	ArgData,
	ArgConst,
	ArgLabel,

	ArgRaw,
	ArgMax,
};

inline ArgEnum& operator++(ArgEnum& arg)
{
	arg = static_cast<ArgEnum>(static_cast<Uint8>(arg) + 1);
	return arg;
}
inline ArgEnum operator++(ArgEnum& arg, int)
{
	ArgEnum old = arg;
	++arg;
	return old;
}

/**
 * Avaiable regs
 */
enum RegEnum : Uint8
{
	RegI0 = 0*sizeof(int),
	RegI1 = 1*sizeof(int),
	RegCond = 2*sizeof(int),

	RegR0 = 3*sizeof(int),
	RegR1 = 4*sizeof(int),
	RegR2 = 5*sizeof(int),
	RegR3 = 6*sizeof(int),

	RegMax = 7*sizeof(int),
};

/**
 * Return value from script operation.
 */
enum RetEnum : Uint8
{
	RetContinue = 0,
	RetEnd = 1,
	RetError = 2,
};

/**
 * Helper class used for calling sequence of function from variadic templates.
 */
struct Dummy
{
	template<typename... ZZ>
	Dummy(ZZ...)
	{
		//nothing
	}
};

/**
 * Struct that cache state of script data and is place of script write temporary data
 */
struct ScriptWorker
{
	const Uint8* proc;
	int shade;
	typename std::aligned_storage<ScriptMaxReg, alignof(void*)>::type reg;

	/// Default constructor
	ScriptWorker() : proc(0), shade(0) //reg not initialized
	{

	}

	/// Programmable bliting using script
	void executeBlit(Surface* src, Surface* dest, int x, int y, bool half = false);
	/// Call script with two arguments
	int execute(int i0, int i1);

	/// Get value from reg
	template<typename T>
	T &ref(size_t offset)
	{
		return *reinterpret_cast<T*>(reinterpret_cast<char*>(&reg) + offset);
	}
	/// Get value from proc vector
	template<typename T>
	const T &const_val(const Uint8 *ptr, size_t offset = 0)
	{
		return *reinterpret_cast<const T*>(ptr + offset);
	}
};

using FuncCommon = RetEnum (*)(ScriptWorker &, const Uint8 *, ProgPos &);

/**
 * Struct used to store definition of used data by script
 */
struct ScriptContainerData
{
	static ArgEnum RegisteImpl()
	{
		static ArgEnum curr = ArgMax;
		return curr++;
	}
	template<typename T>
	static ArgEnum Register()
	{
		if (std::is_same<T, int>::value)
		{
			return ArgReg;
		}
		else
		{
			static ArgEnum curr = RegisteImpl();
			return curr;
		}
	}

	ArgEnum type;
	int index;

	int value;
};

/**
 * Common base of script execution
 */
class ScriptContainerBase
{
	friend class ScriptParserBase;
	std::vector<Uint8> _proc;

protected:
	/// Protected destructor
	~ScriptContainerBase() { }

	void updateBase(ScriptWorker* ref) const
	{
		memset(&ref->reg, 0, ScriptMaxReg);
		if (*this)
		{
			ref->proc = this->_proc.data();
		}
		else
		{
			ref->proc = 0;
		}
	}

public:
	/// constructor
	ScriptContainerBase() = default;
	/// copy constructor
	ScriptContainerBase(const ScriptContainerBase&) = delete;
	/// move constructor
	ScriptContainerBase(ScriptContainerBase&&) = default;

	/// copy
	ScriptContainerBase &operator=(const ScriptContainerBase&) = delete;
	/// move
	ScriptContainerBase &operator=(ScriptContainerBase&&) = default;

	/// Test if is any script there
	explicit operator bool() const
	{
		return !_proc.empty();
	}
};

/**
 * Strong typed script
 */
template<typename... Args>
class ScriptContainer : public ScriptContainerBase
{
	template<size_t offset, typename First, typename... Rest>
	void setReg(ScriptWorker* ref, First f, Rest... t) const
	{
		ref->ref<First>(offset) = f;
		setReg<offset + sizeof(First), Rest...>(ref, t...);
	}
	template<size_t offset>
	void setReg(ScriptWorker* ref) const
	{
		//end loop
	}
public:
	/// Update values in script
	void update(ScriptWorker* ref, Args... args) const
	{
		updateBase(ref);
		if (*this)
		{
			setReg<RegMax>(ref, args...);
		}
	}
};

/**
 * Struct storing avaliable operation to scripts
 */
struct ScriptParserData
{
	using argFunc = int (*)(ParserHelper& ph, const SelectedToken *begin, const SelectedToken *end);
	using getFunc = FuncCommon (*)(int version);
	using parserFunc = bool (*)(const ScriptParserData &spd, ParserHelper &ph, const SelectedToken *begin, const SelectedToken *end);

	parserFunc parser;
	argFunc arg;
	getFunc get;

	bool operator()(ParserHelper &ph, const SelectedToken *begin, const SelectedToken *end) const
	{
		return parser(*this, ph, begin, end);
	}
};

/**
 * Common base of script parser
 */
class ScriptParserBase
{
	Uint8 _regUsed;
	std::string _name;
	std::map<std::string, std::pair<ArgEnum, size_t>> _typeList;
	std::map<std::string, ScriptParserData> _procList;
	std::map<std::string, ScriptContainerData> _refList;

protected:

	/// Default constructor
	ScriptParserBase(const std::string& name);

	/// Common typeless part of parsing string
	bool parseBase(ScriptContainerBase* scr, const std::string& parentName, const std::string& code) const;
	/// Show all builtin script informations
	void logScriptMetadata() const;
	/// Get name of type
	const std::string& getTypeName(ArgEnum type) const;

	/// Add name for standart reg
	void addStandartReg(const std::string& s, RegEnum index);
	/// Add name for custom parameter
	void addCustomReg(const std::string& s, ArgEnum type, size_t size);
	/// Add parsing fuction
	void addParserBase(const std::string& s, ScriptParserData::argFunc arg, ScriptParserData::getFunc get);
	/// Add new type impl
	void addTypeBase(const std::string& s, ArgEnum type, size_t size);
public:

	/// Add const value
	void addConst(const std::string& s, int i);
	// Add parsing function
	template<typename T>
	void addParser(const std::string& s)
	{
		addParserBase(s, &T::parse, &T::getDynamic);
	}
	/// Add new type
	template<typename T>
	void addType(const std::string& s)
	{
		addTypeBase(s, ScriptContainerData::Register<T>(), sizeof(T));
	}
};

/**
 * Strong typed parser
 */
template<typename T, typename... Args>
class ScriptParser : public ScriptParserBase
{
	template<typename Z>
	struct S
	{
		S(const char *n) : name{ n } { }
		S(const std::string& n) : name{ n.data() } { }

		const char *name;
	};
	template<typename First, typename... Rest>
	void addReg(S<First>& n, Rest&... t)
	{
		addCustomReg(n.name, ScriptContainerData::Register<First>(), sizeof(First));
		addReg(t...);
	}
	void addReg()
	{
		//end loop
	}
public:
	using Container = ScriptContainer<Args...>;


	/// Default constructor
	ScriptParser(const std::string& name, S<Args>... argNames) : ScriptParserBase(name)
	{
		//ScriptParser require static function in T to initialize data!
		T::ScriptRegister(this);
		addReg(argNames...);
	}

	/// Prase string and return new script
	Container parse(const std::string& parentName, const std::string& srcCode) const
	{
		auto scr = Container{};
		if (parseBase(&scr, parentName, srcCode))
		{
			return std::move(scr);
		}
		return {};
	}

	/// Print data to log
	void LogInfo() const
	{
		static bool printOp = [this]{ logScriptMetadata(); return true; }();
		(void)printOp;
	}
};

} //namespace OpenXcom

#endif	/* OPENXCOM_SCRIPT_H */

