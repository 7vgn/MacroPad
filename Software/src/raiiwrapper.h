/**
 * \file raiiwrapper.h
 * This file defines a wrapper class that makes C-style objects behave in a
 * proper and C++-appropriate RAII fashion.
 */

#ifndef _RAIIWRAPPER_H
#define _RAIIWRAPPER_H

#include<functional>

/**
 * \brief Helper class for automatically freeing C-style objects when they go
 * out of scope
 * \details When working with C-style "objects" that need to be freed once
 * allocated, using exceptions becomes cumbersome since C++ does not offer a
 * Java-like "finally". This class wraps around such an object and
 * automatically frees it when it goes out of scope.
 * \code
 * void myFunc()
 * {
 * 	RaiiWrapper<FILE*> myFile(fopen("myFile.name", "r"), [](FILE* f){fclose(f);})
 * 	//...
 * 	char c = fgetc(myFile);
 * 	//...
 * 	if(somethingWentWrong)
 * 		throw runtime_error("Help!");
 * 	//...
 * } // The file is closed automatically, regardless of how myFunc() is left.
 * \endcode
 */
template<typename Handle>
class RaiiWrapper
{
private:
	/**
	 * \brief Handle of the "object"
	 */
	Handle handle;

	/**
	 * \brief Function that needs to be called when this instance dies
	 */
	std::function<void(Handle)> finalizer;

public:
	/**
	 * \brief Constructs a RaiiWrapper
	 * \param handle Handle to the "objects" that needs to be finalized eventually.
	 * \param finalizer Function that is called when this instance does out of
	 * scope.
	 */
	RaiiWrapper(Handle handle, std::function<void(Handle)> finalizer): handle(handle), finalizer(finalizer) {}

	/**
	 * \brief Destructor that calls the finalize() function.
	 */
	~RaiiWrapper() {finalizer(handle);}

	/**
	 * \brief Instances are not copy-constructable
	 */
	RaiiWrapper(const RaiiWrapper<Handle>&) = delete;

	/**
	 * \brief Instances are not copy-assignable
	 */
	RaiiWrapper<Handle>& operator=(const RaiiWrapper<Handle>&) = delete;

	/**
	 * \brief Access to handle
	 * \details This makes it so the wrapper can be used in place of the
	 * original object.
	 * \returns The handle to the "object".
	 */
	operator Handle() {return handle;}
};

/**
 * \brief Special case of RaiiWrapper when there is no tangible "object"
 * \code
 * void myFunc()
 * {
 * 	initLibrary();
 * 	RaiiWrapper<void> myLibrary([](){finaliseLibrary();})
 * 	//...
 * 	if(somethingWentWrong)
 * 		throw runtime_error("Help!");
 * 	//...
 * } // The library is finalised automatically, regardless of how myFunc() is left.
 * \endcode
 */
template<>
class RaiiWrapper<void>
{
private:
	/**
	 * \brief Function that needs to be called when this instance dies
	 */
	std::function<void()> finalizer;

public:
	/**
	 * \brief Constructs a RaiiWrapper
	 * \param finalizer Function that is called when this instance does out of
	 * scope.
	 */
	RaiiWrapper(std::function<void()> finalizer): finalizer(finalizer) {}

	/**
	 * \brief Destructor that calls the finalize() function.
	 */
	~RaiiWrapper() {finalizer();}

	/**
	 * \brief Instances are not copy-constructable
	 */
	RaiiWrapper(const RaiiWrapper<void>&) = delete;

	/**
	 * \brief Instances are not copy-assignable
	 */
	RaiiWrapper<void>& operator=(const RaiiWrapper<void>&) = delete;
};

#endif // _RAIIWRAPPER_H
