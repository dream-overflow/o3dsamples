/**
 * @file minimal.cpp
 * @brief Main entry of the minimal sample.
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2004-01-01
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include <o3d/core/architecture.h>
#include <o3d/core/matrix4.h>
#include <o3d/core/wintools.h>
#include <o3d/core/application.h>
#include <o3d/core/md5.h>
#include <o3d/core/sha1.h>
#include <o3d/core/uuid.h>
#include <o3d/core/main.h>
#include <o3d/core/dynamiclibrary.h>

#include <math.h>

/** You only just to define a like-main function for your program and to call it with O3D_APPLICATION
  * Application::init() and Application::quit are automatically called with him.
  *
  * Use O3D_CONSOLE to enable the console support on Windows systems.
  *
  * Otherwise, you can use your proper mechanism but you should call Application::init() at program
  * startup and Application::quit at program exit.
  *
  * Example of multi-platform implementation:

		#ifdef _MSC_VER
		Int32 APIENTRY WinMain(HINSTANCE hinstance,
							   HINSTANCE hPrevinstance,
							   LPSTR lpCmdLine,
							   Int32 nCmdShow)
		{
		#else
		Int32 main(int argc, char **argv)
		{
		#endif
			// Initialisation
			Application::init();

			// Draw a hello world message
			Application::message("", "Hello world!");

			// Destruction
			Application::quit();

			return 0;
		}
*/

#include <o3d/core/debug.h>
#include <o3d/core/string.h>
#include <o3d/core/processor.h>
#include <o3d/core/commandline.h>
#include <o3d/core/math.h>
#include <o3d/core/virtualfilelisting.h>

using namespace o3d;

/*
WaitCondition waitcond;
FastMutex mutex;

// Test for wait condition
int call(void*data)
{
	int time = *(int*)data;

	mutex.lock();

	waitcond.Wait(mutex);

	mutex.unlock();

	System::waitMs(time);
	Application::message("", "check");
	return 0;
}
*/

#include <o3d/core/filemanager.h>
#include <o3d/core/localdir.h>
#include <o3d/core/stringutils.h>

inline Float inl_sqrt(Float x)
{
#if defined(O3D_IX32) || defined(O3D_IX64)
  #ifdef _MSC_VER
    __asm
    {
        fld     x
        fsqrt
    }
  #elif defined(__GNUC__)
    register Float ret;
    asm("fsqrt" : "=t"(ret): "0"(x));
    return ret;
  #else
    return ::sqrtf(x);
  #endif
#else
    return ::sqrtf(x);
#endif
}

inline Float inl_sse_sqrt(Float x)
{
#ifdef O3D_SSE2
  #if defined(_MSC_VER)
    static Float half = 0.5f;
    static Float three = 3.0f;
    Float y;  // = 0.f; not set to 0 otherwise optimization will take 0 as result of the function

    __asm
    {
        movss     xmm3, x     // xmm3 = (x, ?, ?, ?)
                movss     xmm4, xmm3   // xmm4 = (x, ?, ?, ?)
                movss     xmm1, half   // xmm1 = (0.5, ?, ?, ?)
                movss     xmm2, three  // xmm2 = (3, ?, ?, ?)
                rsqrtss   xmm0, xmm3   // xmm0 = (~ 1 / sqrt(x), ?, ?, ?)
                mulss     xmm3, xmm0   // xmm3 = (~ sqrt(x), ?, ?, ?)
                mulss     xmm1, xmm0   // xmm1 = (~ 0.5 / sqrt(x), ?, ?, ?)
                mulss     xmm3, xmm0   // xmm3 = (~ 1, ?, ?, ?)
                subss     xmm2, xmm3   // xmm2 = (~ 2, ?, ?, ?)
                mulss     xmm1, xmm2   // xmm1 = (~ 1 / sqrt(x), ?, ?, ?)
                mulss     xmm1, xmm4   // xmm1 = (sqrt(x), ?, ?, ?)
                movss     y, xmm1      // store result
    }

    return y;
  #elif defined(__GNUC__)
    static Float half = 0.5f;
    static Float three = 3.0f;
    Float y;   // = 0.f; not set to 0 otherwise optimization will take 0 as result of the function

    __asm__ __volatile__ ("movss %0,%%xmm3 \n\t" : : "m" (x));
    __asm__ __volatile__ ("movss %xmm3,%xmm4 \n\t");
    __asm__ __volatile__ ("movss %0,%%xmm1 \n\t" : : "m" (half));
    __asm__ __volatile__ ("movss %0,%%xmm2 \n\t" : : "m" (three));
    __asm__ __volatile__ ("rsqrtss %xmm3,%xmm0 \n\t");
    __asm__ __volatile__ ("mulss %xmm0,%xmm3 \n\t");
    __asm__ __volatile__ ("mulss %xmm0,%xmm1 \n\t");
    __asm__ __volatile__ ("mulss %xmm0,%xmm3 \n\t");
    __asm__ __volatile__ ("subss %xmm3,%xmm2 \n\t");
    __asm__ __volatile__ ("mulss %xmm2,%xmm1 \n\t");
    __asm__ __volatile__ ("mulss %xmm4,%xmm1 \n\t");
    __asm__ __volatile__ ("movss %%xmm1,%0" : : "m" (y));

    // __asm__ __volatile__ ("sqrtss %xmm3,%xmm0 \n\t");
    // __asm__ __volatile__ ("movss %%xmm0,%0" : : "m" (y));

    return y;
  #else
    return ::sqrt(x);
  #endif
#else
    return ::sqrtf(x);
#endif
}

// Main class
class Minimal {

public:

	// main entry
	static Int32 main()
	{
        // Single app instance
		if (Application::isMappedFileExists("o3d_minimal")) {
			Application::message("An instance already running", "");
			return 0;
		}

		Application::mapSingleFile("o3d_minimal");

		// If you want to check processor information
		Processor proc;
		// And to log it...
		proc.reportLog();

		// Test of dynamic library loading and calling
        #ifdef O3D_WINDOWS
        DynamicLibrary *lib = DynamicLibrary::load("libdynlib.dll");
        #else
		DynamicLibrary *lib = DynamicLibrary::load("libdynlib.so");
        #endif

		// C style
		{
			typedef int Fooo(int,int);
			Fooo *foooo = (Fooo*)lib->getFunctionPtr("fooo");
            System::print(String("libdynlib.so::foo found @0x{0}").arg(UInt64(foooo), 16, 16, '0'), "");
			System::print(String::print("CStyle libdynlib.so::foo %i", foooo(10,5)), "");
		}

        // C++ Style
        {
            auto foooo = lib->getFunction<int,int,int>("fooo");
            System::print(String::print("C++Style libdynlib.so::foo %i", foooo(10,5)), "");
        }

		DynamicLibrary::unload(lib);

		// Draw a hello world message
		Application::message("Hello world!", "minimal");
		Application::message(Application::getAppName(), "My name is");
		Application::message(Application::getAppPath(), "And I'm located at");

		// Md5 digest
        MD5Hash md5;
        CString toHash("Hash me this string");
        md5.update(toHash.getBytes());
        md5.finalize();
        Application::message(md5.getHex(), String("I'm the MD5 for ") + toHash);

        // Sha1 digest
        SHA1Hash sha1;
        sha1.update(toHash.getBytes());
        sha1.finalize();
        Application::message(sha1.getHex(), String("I'm the SHA1 for ") + toHash);

        // Uuid 1,3 and 5
        Uuid uuid = Uuid::uuid1();
        Application::message(uuid.toString(), String::print("I'm an UUID version %i", uuid.version()));

        uuid = Uuid::uuid3();
        Application::message(uuid.toString(), String::print("I'm an UUID version %i", uuid.version()));

        uuid = Uuid::uuid5();
        Application::message(uuid.toString(), String::print("I'm an UUID version %i", uuid.version()));

        // Utf8 conversion
		String c(L"Unicode string àéèêË");
		CString cc = c.toUtf8();
		c.fromUtf8(cc.getData());

        Application::message(c, "minimal");

        // Arg string
        Application::message(String("I'm the {0} and I'm {1}/{2}!").arg("World").arg(4.9f, 1).arg(5), "Arg string");

        // Smart array to string
        String d(StringUtils::toHex(SmartArrayUInt8((const UInt8*)"abcdefghijklm", 13)));
        System::print(d, "'abcdefghijklm' -> toHex");

        String arrayStr;

		ArrayUInt32 arr;
		arr.push(3);
		arr.push(4);
		arr.push(5);
		arr.push(1);
		arr.push(2);
		arr.push(9);
		arr.push(8);
		arr.push(7);
		arr.push(6);
		arr.push(10);
		arr.push(11);
		arr.sort();
        for (int i =0;i<11;++i)
        {
            arrayStr.concat(arr[i]);
            arrayStr += ' ';
        }

        System::print(arrayStr, "display array content", System::MSG_DEBUG);

	/*
		// Test relative path, make absolute, cd up and finalize with a make full path
		// output must be /something/api/shaders
        Dir ls("./");
		ls.makeAbsolute();
		ls.cdUp();
		Application::message(ls.MakeFullPathName("shaders"), "");
		// ls.removeDir("shaders");
	*/
	/*
		// Create a file with unicode chars
        FileOutStream fileos;
        fileos.open(L"kékàze");
        fileos.close();
	*/
	/*
		// Test for read the first line of text of '.xml' file from a 'xml.pack' (zip) file found
		// in the working directory and containing a directory named 'xml' with many *.xml files
		FileManager::instance()->addPackFile("xml.pack");

		VirtualFileListing fileListing;
		fileListing.setPath("./xml");
        fileListing.setType(FILE_FILE);
		fileListing.setExt("*.xml");
		fileListing.searchFirstFile();

		FLItem *Item;
        while ((Item = fileListing.searchNextFile()) != nullptr) {
			Application::message(Item->FileName, "minimal");
            File *file = FileManager::instance()->openFile(fileListing.getFileFullName(), "rt");
			String line;
            file->readLine(line);
			Application::message(line, "minimal");
			deletePtr(file);
		}
	*/
	/*
		// Test for wait condition
		Thread t1, t2;
		int time1=5000, time2=10000;

		t1.create(new CallbackFunction(call),(void*)&time1);
		t2.create(new CallbackFunction(call),(void*)&time2);

		waitcond.wakeAll();

		t1.waitFinish();
		t2.waitFinish();

		time1 = time2;
	*/
	/*
		// Test for command line parser
		CommandLine *commandLine = Application::commandLine();
		
		commandLine->registerArgument("file");
		commandLine->addSwitch('t');
		commandLine->addOption("verbose");
		commandLine->addOption("inc");
		commandLine->addVarLenOption("Rep");
		commandLine->addRepeatableOption('D');
		commandLine->addOptionalOption('o',"","1");
		commandLine->addOptionalOption('O',"","2");
		commandLine->addOptionalOption("opt","4");
		commandLine->addOptionalOption("opt2","8");
		commandLine->parse();
		// test with: --Rep v1 v2 v3 --opt --inc="my value" --opt2=15 -O -o150 -D/prout -D/pwet -t --verbose=0 "my text \"@\""
	*/
	/*
		// Simple mutex test
		Mutex mu;
		if (mu.lock(500)) {
			printf("Mutex Locked!\n");
		} else {
			printf("Bad lock!\n");
		}
		mu.unlock();
		printf("Unlocked!\n");

		// Recursive mutex test
		RecursiveMutex mf;
		mf.lock();
		printf("hello!\n");
		mf.lock();
		printf("reHello!\n");
		mf.unlock();
		printf("bye!\n");
		mf.unlock();
		printf("reBye!\n");
	*/
		Int64 timer;

		// Test of differents sqrt methods
		Float r;

		timer = System::getTime();
		for (int i=0;i<10000000;i+=1) {
			r = Math::_Std::sqrt((Float)i);
		}
		Float time = (Float)(System::getTime() - timer) / (Float)System::getTimeFrequency();
		Application::message(String::print("Math::_Std::sqrt %f // time %f",r, time), "Bench");

		timer = System::getTime();
		for (int j=0;j<10000000;j+=1) {
			r = Math::_SSE::sqrt((Float)j);
		}
		time = (Float)(System::getTime() - timer) / (Float)System::getTimeFrequency();
		Application::message(String::print("Math::_SSE::sqrt %f // time %f",r, time), "Bench");

		timer = System::getTime();
		for (int j=0;j<10000000;j+=1) {
			r = ::sqrt((Double)j);
		}
		time = (Float)(System::getTime() - timer) / (Float)System::getTimeFrequency();
		Application::message(String::print("::sqrt %f // time %f",r, time), "Bench");

		timer = System::getTime();
		for (int j=0;j<10000000;j+=1) {
			r = ::sqrtf((Float)j);
		}
		time = (Float)(System::getTime() - timer) / (Float)System::getTimeFrequency();
		Application::message(String::print("::sqrtf %f // time %f",r, time), "Bench");

		timer = System::getTime();
		for (int j=0;j<10000000;j+=1) {
            r = inl_sqrt((Float)j);
		}
		time = (Float)(System::getTime() - timer) / (Float)System::getTimeFrequency();
		Application::message(String::print("inl_sqrt %f // time %f",r, time), "Bench");

		timer = System::getTime();
		for (int j=0;j<10000000;j+=1) {
			r = inl_sse_sqrt((Float)j);
		}
		time = (Float)(System::getTime() - timer) / (Float)System::getTimeFrequency();
		Application::message(String::print("inl_sse_sqrt %f // time %f",r, time), "Bench");

		timer = System::getTime();
		for (int j=0;j<10000000;j+=1) {
			r = Math::rsqrt((float)j);
		}
		time = (Float)(System::getTime() - timer) / (Float)System::getTimeFrequency();
		Application::message(String::print("Math::rsqrt %f // time %f",r, time), "Bench");

	/*
		// Test of the block memory allocator
		void** myArray = new void*[65536];

		for (UInt32 i = 0; i < 65536; ++i) {
			myArray[i] = O3D_FAST_ALLOC(64);
		}

		for (UInt32 i = 0; i < 65536; ++i) {
			O3D_FAST_FREE(myArray[i],64);
			myArray[i] = nullptr;
		}

		deleteArray(myArray);
	*/
	/*
		// Test of the exception manager
		String test;

		try {
			O3D_ERROR(O3D_E_InvalidFormat("Simulate a crash!"));
		} catch(O3D_E_BaseException e) {
			test = e.getDescr();
		}

		//if (0) O3D_ERROR(O3D_E_InvalidFormat("Simulate another crash!"));
	*/
	/*
		// Test for matrix products and matrix to string conversion
		Matrix4 a(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16),b(16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1),c;
		a *= b; c = a;
		//c = a*b;

		Apps::message("a*b=" + (String)c);
	*/
	/*
		// Matrix product test and fast inverse sqrt
		Matrix4 *mm = new Matrix4[13];
		Matrix4 a(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16),b(16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1),c;
		Matrix4 d(0,1,0,36,0,0,1,45,1,0,0,12,0,0,0,1);
		mm[0] = a;
		mm[1] = b;
		mm[2] = c;

		timer = System::getTime();
		for(unsigned int j = 0 ; j < 20000000; ++j) {
			a.mult(b,mm[0]);
			mm[0] *= c;
		}
		Float time = ((System::getTime() - timer) * 1000000) / System::getTimeFrequency() / 1000000.f;
		Apps::message(String::print("transpose in %f seconds! a=",time) + String(mm[2]) << invSqrt(0.00137), "minimal");
	*/
	/*
		// Matrix allocation and product test
		timer = System::getTime();
		Matrix4 mm;

		for (int j = 0 ; j < 5000000; ++j) {
			Matrix4 a,b,c,d,e,f,g;//,h,i,jj,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z;
			mm = a*b*c*d*e*f*g;
		}
		Float time2 = ((System::getTime() - timer) * 1000000) / System::getTimeFrequency() / 1000000.f;

		Application::message(String::print("Bench %f seconds!",time2));// + String(mm[2]), "minimal");
	*/
	
		// Mutex vs FastMutex bench
		timer = System::getTime();

		FastMutex fm;

		for (int j = 0 ; j < 5000000; ++j) {
			fm.lock();
			fm.unlock();
		}
		
		time = ((System::getTime() - timer) * 1000000) / System::getTimeFrequency() / 1000000.f;
		Application::message(String::print("Fast Mutex %f'", time), "Bench");

		timer = System::getTime();

		Mutex mu;

		for (int j = 0 ; j < 5000000; ++j) {
			mu.lock();
			mu.unlock();
		}
		
		time = ((System::getTime() - timer) * 1000000) / System::getTimeFrequency() / 1000000.f;
		Application::message(String::print("Mutex %f'",time), "Bench");

		return 0;
	}
};

class MyAppSettings : public AppSettings
{
public:

    MyAppSettings() : AppSettings()
    {
        sizeOfFastAlloc16 = 16384;
        sizeOfFastAlloc32 = 16384;
        sizeOfFastAlloc64 = 16384;
        useDisplay = False;
        clearLog = True;
    }
};

// We Call our application using the defaults settings
O3D_CONSOLE_MAIN(Minimal, MyAppSettings)  // O3D_DEFAULT_CLASS_SETTINGS)
