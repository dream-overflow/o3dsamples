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
			Application::message(NULL,"Hello world!");

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
O3DWaitCondition waitcond;
O3DFastMutex mutex;

// Test for wait condition
int call(void*data)
{
	int time = *(int*)data;

	mutex.Lock();

	waitcond.Wait(mutex);

	mutex.Unlock();

	O3DSystem::Wait(time);
	O3D_MessageBox(NULL,"check");
	return 0;
}
*/
#include <o3d/core/filemanager.h>
#include <o3d/core/diskdir.h>
#include <o3d/core/stringutils.h>
//#include <o3d/engine/animation/AnimationTrack.h>

// Main class
class Minimal {

public:

	// main entry
	static Int32 main()
	{
		if (Application::isMappedFileExists("o3d_minimal"))
		{
			Application::message("An instance already running", "");
			return 0;
		}

		Application::mapSingleFile("o3d_minimal");

		// Set the default log filename
        Debug::instance()->setDefaultLog("minimal.log");
		// Clear the output log file
		Debug::instance()->getDefaultLog().clearLog();
		// Write the standard O3D banner into the log file
		Debug::instance()->getDefaultLog().writeHeaderLog();

		// If you want to check processor information
		Processor proc;
		// And to log it...
		proc.reportLog();

		// Test of dynamic library loading and calling
		DynamicLibrary *lib = DynamicLibrary::load("libdynlib.so");

		// C++ style
		{
			auto foooo = lib->getFunction<int,int,int>("fooo");
			System::print(String::print("C++Style libdynlib.so::foo %i", foooo(10,5)), "");
		}

		// C style
		{
			typedef int Fooo(int,int);
			Fooo *foooo = (Fooo*)lib->getFunctionPtr("fooo");
			System::print(String::print("CStyle libdynlib.so::foo %i", foooo(10,5)), "");
		}

		DynamicLibrary::unload(lib);

		// Draw a hello world message
		Application::message("Hello world!", "minimal");
		Application::message(Application::getAppName(), "My name is");
		Application::message(Application::getAppPath(), "And I'm located at");

        MD5Hash md5;
        CString toHash("Hash me this string");
        md5.update(toHash.getBytes());
        md5.finalize();
        Application::message(md5.getHex(), String("I'm the MD5 for ") + toHash);

        SHA1Hash sha1;
        sha1.update(toHash.getBytes());
        sha1.finalize();
        Application::message(sha1.getHex(), String("I'm the SHA1 for ") + toHash);

        Uuid uuid = Uuid::uuid1();
        Application::message(uuid.toString(), String::print("I'm an UUID version %i", uuid.version()));

        uuid = Uuid::uuid3();
        Application::message(uuid.toString(), String::print("I'm an UUID version %i", uuid.version()));

        uuid = Uuid::uuid5();
        Application::message(uuid.toString(), String::print("I'm an UUID version %i", uuid.version()));

		String c(L"Unicode string àéèêË");
		CString cc = c.toUtf8();
		c.fromUtf8(cc.getData());

        Application::message(c, "minimal");

        Application::message(String("This %1, %2 and %3").arg(1).arg("two").arg(3.01f).result(), "arg string");

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
		test teste;

		O3DMainWindow::instance()->SetWindow(640,480);
		O3DMainWindow::instance()->Run();*/
	/*
		// Test relative path, make absolute, cd up and finalize with a make full path
		// output must be /something/api/shaders
		O3DDiskDir ls("./");
		ls.MakeAbsolute();
		ls.CdUp();
		O3DApps::Message(ls.MakeFullPathName("shaders"),"");
	*/
	/*
		// Create a file with unicode chars
		O3DDiskFile filed;
		filed.Open(L"kékàze","wb");
		filed.Close();
	*/

	/*
		// Test for read the first line of text of '.xml' file from a 'xml.pack' (zip) file found
		// in the working directory and containing a directory named 'xml' with many *.xml files
		O3DFileManager::instance()->AddPackFile("xml.pack");

		O3DVirtualFileListing fileListing;
		fileListing.SetPath("./xml");
		fileListing.SetType(O3DFileFile);
		fileListing.SetExt("*.xml");
		fileListing.SearchFirstFile();

		O3DFLItem *Item;
		while ((Item = fileListing.SearchNextFile()) != NULL)
		{
			O3DApps::Message(Item->FileName, "minimal");
			O3DFile *file = O3DFileManager::instance()->OpenFile(fileListing.GetFileFullName(), "rt");
			O3DString line;
			file->ReadLine(line);
			O3DApps::Message(line, "minimal");
			deletePtr(file);
		}
	*/

	/*
		// Test for wait condition
		O3DThread t1,t2;
		int time1=5000,time2=10000;

		t1.Create(new O3DCallbackFunction(call),(void*)&time1);
		t2.Create(new O3DCallbackFunction(call),(void*)&time2);

		waitcond.WakeAll();

		t1.WaitFinish();
		t2.WaitFinish();

		time1=time2;
	*/

	/*
		// Test for command line parser
		O3DCommandLine *commandLine = O3DApps::CommandLine();
		commandLine->RegisterArgument("file");
		commandLine->AddSwitch('t');
		commandLine->AddOption("verbose");
		commandLine->AddOption("inc");
		commandLine->AddVarLenOption("Rep");
		commandLine->AddRepeatableOption('D');
		commandLine->AddOptionalOption('o',"","1");
		commandLine->AddOptionalOption('O',"","2");
		commandLine->AddOptionalOption("opt","4");
		commandLine->AddOptionalOption("opt2","8");
		commandLine->Parse();
		test with: --Rep v1 v2 v3 --opt --inc="my sound" --opt2=15 -O -o150 -D/prout -D/pwet -t --verbose=0 "my texte \"@\""
		*/

	/*
		// Simple mutex test
		O3DMutex mu;
		if (mu.Lock(500))
			printf("Locked!\n");
		else
			printf("Bad lock!\n");
		mu.Unlock();
		printf("Unlocked!\n");


		// Recursive mutex test
		O3DRecursiveMutex mf;
		mf.Lock();
		printf("hello!\n");
		mf.Lock();
		printf("reHello!\n");
		mf.Unlock();
		printf("bye!\n");
		mf.Unlock();
		printf("reBye!\n");
	*/

	/*
		// Test of differents sqrt methods
		Float r;

		Int64 timer = O3DSystem::GetTime();
		for (int i=0;i<10000000;i+=1)
			r = O3DMath::_Std::Sqrt((float)i);
		Float time = (Float)(O3DSystem::GetTime() - timer) / (Float)O3DSystem::GetTimeFrequency();
		printf("%f // %f\n",r,time);

		timer = O3DSystem::GetTime();
		for (int j=0;j<10000000;j+=1)
			r = O3DMath::_SSE::Sqrt((float)j);
		time = (Float)(O3DSystem::GetTime() - timer) / (Float)O3DSystem::GetTimeFrequency();
		printf("%f // %f\n",r,time);
	*/

	/*
		// Test of the block memory allocator
		void** myArray = new void*[65536];

		for (UInt32 i = 0; i < 65536; ++i)
		{
			myArray[i] = O3D_FAST_ALLOC(64);
		}

		for (UInt32 i = 0; i < 65536; ++i)
		{
			O3D_FAST_FREE(myArray[i],64);
			myArray[i] = NULL;
		}

		deleteArray(myArray);
	*/

	/*
		// Test of the exception manager
		O3DString test;

		try
		{
			O3D_ERROR(O3D_E_InvalidFormat("Simulate a crash!"));
		}
		catch(O3D_E_BaseException e)
		{
			test = e.GetDescr();
		}

		//if (0) O3D_ERROR(O3D_E_InvalidFormat("Simulate another crash!"));
	*/

	/*
		// Test for matrix products and matrix to string conversion
		O3DMatrix4 a(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16),b(16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1),c;
		a *= b; c = a;
		//c = a*b;

		O3DApps::Message("a*b=" + (O3DString)c);
	*/

	/*
		// Matrix product test and fast inverse sqrt
		O3DMatrix4 *mm = new O3DMatrix4[13];
		O3DMatrix4 a(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16),b(16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1),c;
		O3DMatrix4 d(0,1,0,36,0,0,1,45,1,0,0,12,0,0,0,1);
		mm[0] = a;
		mm[1] = b;
		mm[2] = c;

		Int64 start = O3DSystem::GetTime();
		for(unsigned int j = 0 ; j < 20000000; ++j)
		{
			a.Mult(b,mm[0]);
			mm[0] *= c;
		}
		Float time = ((O3DSystem::GetTime() - start) * 1000000) / O3DSystem::GetTimeFrequency() / 1000000.f;
		O3DApps::Message(O3DString::Print("transpose in %f seconds! a=",time) + O3DString(mm[2]) << InvSqrt(0.00137), "minimal");

		return 0;
	*/

	/*
		// Matrix allocation and product test
		Int64 start = O3DSystem::GetTime();
		O3DMatrix4 mm;

	//	O3DFastMutex fm;

		for (int j = 0 ; j < 5000000; ++j)
		{
	//		fm.Lock();
	//		fm.Unlock();
	//		fm.Lock();
	//		fm.Unlock();
	//		fm.Lock();
	//		fm.Unlock();
			O3DMatrix4 a,b,c,d,e,f,g;//,h,i,jj,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z;
			mm = a*b*c*d*e*f*g;
		}
		Float time2 = ((O3DSystem::GetTime() - start) * 1000000) / O3DSystem::GetTimeFrequency() / 1000000.f;

		O3DApps::Message(O3DString::Print("Bench %f seconds!",time2));// + O3DString(mm[2]), "minimal");
	*/

		// Write the standard O3D closer banner into the log file
		Debug::instance()->getDefaultLog().writeFooterLog();

		return 0;
	}
};

// We Call our application using the defaults settings
O3D_CONSOLE_MAIN(Minimal, O3D_DEFAULT_CLASS_SETTINGS)

