#pragma once

namespace sge {

#define PathStatus_ENUM_LIST(E) \
	E(None,) \
	E(Exist,) \
	E(NotExist,) \
	E(Directory,) \
	E(File,) \
//----
SGE_ENUM_CLASS(PathStatus, u8);

struct Path
{
	Path() = delete;
	using Status = PathStatus;

	static Status	status(const StrView& path_);

	static bool		isFile(const StrView& path_);
	static bool		isDirectory(const StrView& path_);
};

struct FilePath {
	static StrView	dirname(StrView path);
	static StrView	basename(StrView path);
	static StrView	extension(StrView path);

	static bool		isRealpath(const StrView& path);
	static String	realpath(StrView path);
};

}