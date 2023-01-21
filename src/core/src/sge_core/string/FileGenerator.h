#pragma once
#include <sge_core/file/FileStream.h>

namespace sge {

class FileGenerator : public NonCopyable
{
public:



private:
	FileStream fs;

};

class GNUMakeGenerator : public NonCopyable
{
public:
	struct RequestBase
	{
		RequestBase(size_t reserveSize = 4096) { _content.reserve(reserveSize); }

		void write(StrView sv)			{  _content.append(sv.data()); }
		//void write(const char* c_str) { content.append(c_str); }

		void nextLine()					{ write("\n"); }
		void writeLine(StrView sv)		{ write(sv); nextLine(); }

		void writeVariable(StrView sv)	{ write("$("); write(sv); write(")"); }

		void onWrite(StrView sv)			
		{ 
			_content.append(_onWriteStr); 
			write(sv); 
		}

		void onWriteVariable(StrView sv)	{ _content.append(_onWriteStr); writeVariable(sv); }

		// cuurently is wrong, should use a stack
		void setOnWriteStr(StrView sv)	{ clearOnWriteStr(); _onWriteStr.assign(sv.data()); }
		void clearOnWriteStr()			{ _onWriteStr.clear(); }

		void flush(StrView filename) { File::writeFileIfChanged(filename, _content, false); }

	private:
		void _onWrite() { write(_onWriteStr); }

	private:
		String _onWriteStr;
		String _content;
	};
private:
	using Request = RequestBase;
protected:
	GNUMakeGenerator(Request& req)
	{
		_GNUMakeGeneratorRequest = &req;
	}

	template<class STR>
	static void var(STR& str, const char* name)	{ str.clear(); str.append("$("); str.append(name); str.append(")"); }

	template<size_t N>
	void phony(const StrView(&names)[N])
	{
		nextLine();
		write(".PHONY:");
		for (size_t i = 0; i < N; i++)
		{
			write(" "); write(names[i]);
		}
		nextLine();
	}

	void assignVariable(StrView var, StrView assign, StrView value) 
	{  
		bool isAssignValid = !(assign.compare(":=") && assign.compare("=")); (void)isAssignValid;
		SGE_ASSERT(isAssignValid);

		nextLine();
		write(var); 
		write(assign); write(value); 
		nextLine();
	}

	void includePath(StrView path)
	{
		//SGE_ASSERT(Directory::exists(path) || FilePath::exists(path));	// should use map to deal with $(var) value
		nextLine();
		write("-include "); write(path.data()); 
		//nextLine();
	}

	void write(StrView sv)				{ auto& req = *_GNUMakeGeneratorRequest; req.write(sv); }
	void onWrite(StrView sv)			{ auto& req = *_GNUMakeGeneratorRequest; req.onWrite(sv); }
	void onWriteVariable(StrView sv)	{ auto& req = *_GNUMakeGeneratorRequest; req.onWriteVariable(sv); }

	void nextLine()					{ auto& req = *_GNUMakeGeneratorRequest; req.nextLine(); }
	void writeLine(StrView sv)		{ auto& req = *_GNUMakeGeneratorRequest; req.writeLine(sv); }
	void writeVariable(StrView sv)	{ auto& req = *_GNUMakeGeneratorRequest; req.writeVariable(sv); }

	void flush(StrView filename)	{ auto& req = *_GNUMakeGeneratorRequest; req.flush(filename);}

	struct SpecialBase
	{
		Request* request = nullptr;

		SpecialBase(Request& request_)
		{
			request = &request_;
		}

		void write(StrView sv)				{ request->write(sv); }
		//void onWrite(StrView sv)			{ request->onWrite(sv); }
		//void onWriteVariable(StrView sv)	{ request->onWriteVariable(sv); }

		void nextLine()					{ request->nextLine(); }
		void writeLine(StrView sv)		{ request->writeLine(sv); }
		void writeVariable(StrView sv)	{ request->writeVariable(sv); }

		void setOnWriteStr(StrView sv)	{ clearOnWriteStr(); request->setOnWriteStr(sv); }
		void clearOnWriteStr()			{ request->clearOnWriteStr(); }
	};

	struct Ifeq : public SpecialBase
	{
		using Base = SpecialBase;
		SGE_NODISCARD static Ifeq ctor(Request& request_, StrView val0, StrView val1) { return Ifeq(request_, val0, val1); }

		~Ifeq()
		{
			clearOnWriteStr();
			nextLine();
			writeLine("endif");
		}

	private:
		Ifeq(Request& request_, StrView var, StrView val)
			: Base(request_)
		{
			request = &request_;
			write("ifeq (");
			writeVariable(var); write(", "); write(val);  write(")");
		}
	};

	struct Target : public SpecialBase
	{
		using Base = SpecialBase;

		template<size_t N>
		SGE_NODISCARD static Target ctor(Request& request_, StrView target, const StrView(&deps)[N])	{ return Target(request_, target, deps); }
		SGE_NODISCARD static Target ctor(Request& request_, StrView target)								{ return Target(request_, target); }

		~Target()
		{
			clearOnWriteStr();
		}
	private:
		template<size_t N>
		Target(Request& request_, StrView target, const StrView(&deps)[N])
			: Base(request_)
		{
			init(request_, target);
			for (size_t i = 0; i < N; i++)
			{
				write(" "); write(deps[i]);
			}
			nextLine();
			setOnWriteStr("\t");
		}

		Target(Request& request_, StrView target)
			: Base(request_)
		{
			init(request_, target);
			setOnWriteStr("\t");
		}

		void init(Request& request_, StrView target)
		{
			request = &request_;
			nextLine();
			write(target.data()); write(":"); 
		}
	};

	struct BeginString : public SpecialBase
	{
		using Base = SpecialBase;
		SGE_NODISCARD static BeginString ctor(Request& request_, StrView str)	{ return BeginString(request_, str); }
		SGE_NODISCARD static BeginString ctor(Request& request_)				{ return BeginString(request_); }

		~BeginString()
		{
			write("\"");
		}

	private:
		BeginString(Request& request_, StrView str)
			: Base(request_)
		{
			request = &request_;
			write("\"");
			write(str);
		}

		BeginString(Request& request_)
			: Base(request_)
		{
			request = &request_;
			write("\"");
		}
	};

	struct BeginCmd : public SpecialBase
	{
		using Base = SpecialBase;
		SGE_NODISCARD static BeginCmd ctor(Request& request_) { return BeginCmd(request_); }

		~BeginCmd()
		{
			//write(" ");
		}

	private:

		BeginCmd(Request& request_)
			: Base(request_)
		{
			request = &request_;
			write(" ");
		}
	};


	Request* _GNUMakeGeneratorRequest = nullptr;
};


}