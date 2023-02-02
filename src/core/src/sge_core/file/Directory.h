#pragma once

#include <filesystem>

namespace sge {

struct Directory {
	Directory() = delete;

	static void setCurrent(StrView dir);
	static String getCurrent();

	static void create(StrView path);
	static bool exists(StrView path);

	static bool isExist(StrView path);
	static bool isDirectory(StrView path);
	static bool isFile(StrView path);

	static void removeIfEmpty(StrView path);
	static void removeAll(StrView path);

private:
	static void _create(StrView path);
};

}