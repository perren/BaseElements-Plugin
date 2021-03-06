/*
 BEPluginFunctions.cpp
 BaseElements Plug-In
 
 Copyright 2010-2011 Goya. All rights reserved.
 For conditions of distribution and use please see the copyright notice in BEPlugin.cpp
 
 http://www.goya.com.au/baseelements/plugin
 
 */


#include "BEPluginGlobalDefines.h"
#include "BEPluginFunctions.h"
#include "BEXSLT.h"
#include "BEWStringVector.h"


#if defined(FMX_WIN_TARGET)

	#include "afxdlgs.h"
	#include <locale.h>

	#include "resource.h"
	#include "BEWinFunctions.h"

	#define PATH_MAX MAX_PATH

	#define POPEN _popen
	#define PCLOSE _pclose

#endif

#if defined(FMX_MAC_TARGET)

	#include "BEMacFunctions.h"

	#define POPEN popen
	#define PCLOSE pclose

#endif

#include "FMWrapper/FMXFixPt.h"
#include "FMWrapper/FMXData.h"
#include "FMWrapper/FMXCalcEngine.h"

#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"

#include <iostream>


using namespace std;
using namespace boost::filesystem;


#pragma mark -
#pragma mark Version
#pragma mark -

FMX_PROC(errcode) BE_Version ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& /* data_vect */, Data& results)
{
	return TextConstantFunction ( VERSION_NUMBER_STRING, results );	
}


FMX_PROC(errcode) BE_VersionAutoUpdate ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& /* data_vect */, Data& results)
{
	return TextConstantFunction ( AUTO_UPDATE_VERSION, results );		
}


#pragma mark -
#pragma mark Clipboard
#pragma mark -

FMX_PROC(errcode) BE_ClipboardFormats ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& /* data_vect */, Data& results)
{
	return TextConstantFunction ( ClipboardFormats(), results );	
}


FMX_PROC(errcode) BE_ClipboardData ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& data_vect, Data& results)
{
	errcode error_result = kNoError;
	
	try {
		
		WStringAutoPtr atype = ParameterAsWideString ( data_vect, 0 );
		StringAutoPtr clipboard_contents = ClipboardData ( atype );
		SetUTF8Result ( clipboard_contents, results );
		
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}
	
	return error_result;
	
} // BE_ClipboardData


FMX_PROC(errcode) BE_SetClipboardData ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& data_vect, Data& results)
{
	errcode error_result = kNoError;
	
	try {

		StringAutoPtr to_copy = ParameterAsUTF8String ( data_vect, 0 );
		WStringAutoPtr atype = ParameterAsWideString ( data_vect, 1 );
		bool success = SetClipboardData ( to_copy, atype );
		SetNumericResult ( success, results );

	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}
	
	return error_result;
	
} // BE_SetClipboardData


#pragma mark -
#pragma mark Files & Folders
#pragma mark -

FMX_PROC(errcode) BE_CreateFolder ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& data_vect, Data& results )
{
	errcode error_result = kNoError;	
	errcode filesystem_result = kNoError;
	
	try {

		WStringAutoPtr folder = ParameterAsWideString ( data_vect, 0 );
		basic_path<wstring, wpath_traits> directory_path = *folder;
		
		try {
			create_directory ( directory_path );
		} catch ( basic_filesystem_error<wpath> e ) {
			filesystem_result = e.code().value();
		}
		
		SetNumericResult ( filesystem_result, results );
				
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}
	
	return error_result;
	
} // BE_CreateFolder


FMX_PROC(errcode) BE_DeleteFile ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& data_vect, Data& results)
{
	errcode error_result = kNoError;
	errcode filesystem_result = kNoError;
	
	try {
		
		WStringAutoPtr file = ParameterAsWideString ( data_vect, 0 );
		basic_path<wstring, wpath_traits> path = *file;
		
		try {
			remove_all ( path ); // if path is a directory then path and all it's contents are deleted
		} catch ( basic_filesystem_error<wpath> e ) {
			filesystem_result = e.code().value();
		}
		
		SetNumericResult ( filesystem_result, results );
		
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}
	
	return error_result;
	
} // BE_DeleteFile


FMX_PROC(errcode) BE_FileExists ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& data_vect, Data& results)
{
	errcode error_result = kNoError;
	
	try {
		
		WStringAutoPtr file = ParameterAsWideString ( data_vect, 0 );
		basic_path<wstring, wpath_traits> path = *file;

		bool file_exists = exists ( path );
		SetNumericResult ( file_exists, results );
		
	} catch ( basic_filesystem_error<wpath> e ) {
		error_result = e.code().value();
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}
	
	return error_result;
	
} // BE_FileExists


FMX_PROC(errcode) BE_ReadTextFromFile ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& data_vect, Data& results)
{
	errcode error_result = kNoError;
	
	try {

		WStringAutoPtr file = ParameterAsWideString ( data_vect, 0 );
		StringAutoPtr contents = ReadFileAsUTF8 ( file );
		SetUTF8Result ( contents, results );

	} catch ( basic_filesystem_error<wpath> e ) {
		error_result = e.code().value();
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}
	
	return error_result;
	
} // BE_ReadTextFromFile


FMX_PROC(errcode) BE_WriteTextToFile ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& data_vect, Data& results)
{
	errcode error_result = kNoError;
		
	try {
		
		WStringAutoPtr file = ParameterAsWideString ( data_vect, 0 );
		basic_path<wstring, wpath_traits> path = *file;
		
		// should the text be appended to the file or replace any existing contents
		
		ios_base::openmode mode = ios_base::trunc;
		if ( data_vect.Size() == 3 ) {
			bool append = data_vect.AtAsBoolean ( 2 );
			if ( append ) {
				mode = ios_base::app;
			}
		}
		
		StringAutoPtr text_to_write = ParameterAsUTF8String ( data_vect, 1 );
		
		try {
			
			basic_path<wstring, wpath_traits> filesystem_path ( path );
			boost::filesystem::ofstream output_file ( filesystem_path, ios_base::out | mode );

			output_file.exceptions ( boost::filesystem::ofstream::badbit | boost::filesystem::ofstream::failbit );			
			
			output_file << *text_to_write;
			output_file.close();

		} catch ( basic_filesystem_error<wpath> e ) {
			error_result = e.code().value();
		} catch ( exception e ) {
			error_result = errno; // unable to write to the file
		}
		
		SetNumericResult ( error_result, results );
		
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}
	
	return error_result;
	
} // BE_WriteTextToFile


/*
 filemaker can create DDRs that contains utf-16 characters that are not
 valid in an XML document. reads the DDR and writes out a new one, skipping
 any invalid characters, and replaces the old file
 */

FMX_PROC(errcode) BE_StripInvalidUTF16CharactersFromXMLFile ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& data_vect, Data& results)
{
	errcode error_result = kNoError;
	
	try {
		
		WStringAutoPtr file = ParameterAsWideString ( data_vect, 0 );
		basic_path<wstring, wpath_traits> source = *file;

		wstring output_path = *file + L".be3.tmp";
		basic_path<wstring, wpath_traits> destination = output_path;
		boost::uintmax_t length = file_size ( source ); // throws if the source does not exist
		
		boost::filesystem::ifstream input_file ( source, ios_base::in | ios_base::binary | ios_base::ate );
		input_file.seekg ( 0, ios::beg );
		boost::filesystem::ofstream output_file ( destination, ios_base::out | ios_base::binary | ios_base::ate );

		const size_t size = 2; // read (and write) 2 bytes at a time
		boost::uintmax_t skipped = 0;
		bool big_endian = true;

		for ( boost::uintmax_t i = 0; i < length; i += size ) {

			char codepoint[size];
			input_file.read ( codepoint, size );
			
			// check the bom, if present, to determin if the utf-16 if big or little endian
			if ( (i == 0) && ((unsigned char)codepoint[0] == 0xff && (unsigned char)codepoint[1] == 0xfe ) ) {
				big_endian = false;
			}
			
			// swap the byte order for big-endian files
			unichar * utf16 = (unichar *)codepoint;
			char byte_swapped[size];
			if ( big_endian ) {
				byte_swapped[0] = codepoint[1];
				byte_swapped[1] = codepoint[0];
				utf16 = (unichar *)byte_swapped;
			}
			
			// only check codepoints in the bmp (so no 4-byte codepoints)
			if ( (*utf16 == 0x9) ||
				(*utf16 == 0xA) ||
				(*utf16 == 0xD) ||
				((*utf16 >= 0x20) && (*utf16 <= 0xD7FF)) ||
				((*utf16 >= 0xE000) && (*utf16 <= 0xFFFE)) ) {
				
				output_file.write ( codepoint, size );

			} else {
				skipped += size;
			}
		}

		output_file.close();
		input_file.close ();
		
		/*
		 only replace the file if that we've skipped some characters and
		 the output file is the right size
		 */
		
		if ( (skipped > 0) && (length == (file_size ( destination ) + skipped)) ) {
			remove_all ( source );
			rename ( destination, source );
		} else {
			remove_all ( destination );
			if ( skipped > 0 ) {
				// if characters were skipped and the file size is wrong report an error
				error_result = kFileSystemError;
			}
		}

		SetNumericResult ( error_result == kNoError, results );
		
	} catch ( basic_filesystem_error<wpath> e ) {
		error_result = e.code().value();
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}
	
	return error_result;
	
} // BE_StripInvalidUTF16CharactersFromXMLFile


FMX_PROC(errcode) BE_MoveFile ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& data_vect, Data& results)
{
	errcode error_result = kNoError;
	errcode filesystem_result = kNoError;
	
	try {
		
		WStringAutoPtr from = ParameterAsWideString ( data_vect, 0 );
		WStringAutoPtr to = ParameterAsWideString ( data_vect, 1 );

		try {
			
			basic_path<wstring, wpath_traits> from_path = *from;
			basic_path<wstring, wpath_traits> to_path = *to;
			
			rename ( from_path, to_path );			
		} catch ( basic_filesystem_error<wpath> e ) {
			filesystem_result = e.code().value();
		}
		
		SetNumericResult ( filesystem_result, results );
		
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}
	
	return error_result;
	
} // BE_MoveFile


FMX_PROC(errcode) BE_CopyFile ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& data_vect, Data& results)
{
	errcode error_result = kNoError;
	errcode filesystem_result = kNoError;
	
	try {
		
		WStringAutoPtr from = ParameterAsWideString ( data_vect, 0 );
		WStringAutoPtr to = ParameterAsWideString ( data_vect, 1 );
		
		try {
			
			basic_path<wstring, wpath_traits> from_path = *from;
			basic_path<wstring, wpath_traits> to_path = *to;
			
			copy_file ( from_path, to_path );
			
		} catch ( basic_filesystem_error<wpath> e ) {
			filesystem_result = e.code().value();
		}
		
		SetNumericResult ( filesystem_result, results );
		
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}
	
	return error_result;
	
} // BE_CopyFile



FMX_PROC(errcode) BE_ListFilesInFolder ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& data_vect, Data& results)
{
	errcode error_result = kNoError;
	
	try {
		
		TextAutoPtr list_of_files;
		TextAutoPtr end_of_line;
		end_of_line->Assign ( "\r" );

		WStringAutoPtr directory = ParameterAsWideString ( data_vect, 0 );
		basic_path<wstring, wpath_traits> directory_path = *directory;
		
		if ( exists ( directory_path ) ) {
				
			basic_directory_iterator<wpath> end_itr; // default construction yields past-the-end
			basic_directory_iterator<wpath> itr ( directory_path );
				
			while ( itr != end_itr ) {
					
				if ( ! is_directory ( itr->status() ) ) {
					TextAutoPtr file_name;
					file_name->AssignWide ( itr->leaf().c_str() );
					list_of_files->AppendText ( *file_name );
					list_of_files->AppendText ( *end_of_line );
				}
				
				++itr;
				
			}

		}

		fmx::LocaleAutoPtr default_locale;
		results.SetAsText ( *list_of_files, *default_locale );
		
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}
	
	return error_result;
	
} // BE_ListFilesInFolder



#pragma mark -
#pragma mark Dialogs
#pragma mark -

FMX_PROC(errcode) BE_SelectFile ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& data_vect, Data& results)
{
	errcode error_result = kNoError;
	
	try {

		WStringAutoPtr prompt = ParameterAsWideString ( data_vect, 0 );
		WStringAutoPtr file = SelectFile ( prompt );
		SetWideResult ( file, results );
		
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}
	
	return error_result;
	
} // BE_SelectFile


FMX_PROC(errcode) BE_SelectFolder ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& data_vect, Data& results)
{
	errcode error_result = kNoError;
	
	try {

		WStringAutoPtr prompt = ParameterAsWideString ( data_vect, 0 );
		WStringAutoPtr folder = SelectFolder ( prompt );
		SetWideResult ( folder, results );
		
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}

	return error_result;
	
} // BE_SelectFolder


FMX_PROC(errcode) BE_DisplayDialog ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& data_vect, Data& results)
{
	errcode error_result = kNoError;
		
	try {

		WStringAutoPtr title = ParameterAsWideString ( data_vect, 0 );
		WStringAutoPtr message = ParameterAsWideString ( data_vect, 1 );
		WStringAutoPtr ok_button = ParameterAsWideString ( data_vect, 2 );
		WStringAutoPtr cancel_button = ParameterAsWideString ( data_vect, 3 );
		WStringAutoPtr alternate_button = ParameterAsWideString ( data_vect, 4 );
	
		int response = DisplayDialog ( title, message, ok_button, cancel_button, alternate_button );
		SetNumericResult ( response, results );

	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}

	return error_result;
	
} // BE_DisplayDialog


// invoked for multiple plug-in functions... funcId is used to determine which one

FMX_PROC(errcode) BE_ButtonConstants ( short funcId, const ExprEnv& /* environment */, const DataVect& /* data_vect */, Data& results)
{
	errcode error_result = kNoError;
	
	try {
		
		SetNumericResult ( funcId - kXMpl_ButtonOffset, results );
		
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}
	
	return error_result;
	
} // BE_ButtonConstants


#pragma mark -
#pragma mark XSLT
#pragma mark -

FMX_PROC(errcode) BE_ApplyXSLT ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& data_vect, Data& results)
{
	errcode error_result = kNoError;
	
	try {

		StringAutoPtr xml_path = ParameterAsUTF8String ( data_vect, 0 );
		StringAutoPtr xslt = ParameterAsUTF8String ( data_vect, 1 );
		StringAutoPtr csv_path = ParameterAsUTF8String ( data_vect, 2 );

		results.SetAsText( *ApplyXSLT ( xml_path, xslt, csv_path ), data_vect.At(0).GetLocale() );
	
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}

	return error_result;
	
} // BE_ApplyXSLT


FMX_PROC(errcode) BE_ApplyXSLTInMemory ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& data_vect, Data& results)
{
	errcode error_result = kNoError;
	
	try {
		
		StringAutoPtr xml = ParameterAsUTF8String ( data_vect, 0 );
		StringAutoPtr xslt = ParameterAsUTF8String ( data_vect, 1 );
		
		results.SetAsText( *ApplyXSLTInMemory ( xml, xslt ), data_vect.At(0).GetLocale() );
		
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}
	
	return error_result;
	
} // BE_ApplyXSLTInMemory


FMX_PROC(errcode) BE_XPath ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& data_vect, Data& results )
{
	errcode error_result = kNoError;
	
	try {
		int numParams = data_vect.Size();
		StringAutoPtr xml = ParameterAsUTF8String ( data_vect, 0 );
		StringAutoPtr xpath = ParameterAsUTF8String ( data_vect, 1 );
		StringAutoPtr nsList(new string);
		if (numParams > 2)
			nsList = ParameterAsUTF8String ( data_vect, 2 );
		
		results.SetAsText( *ApplyXPath ( xml, xpath, nsList ), data_vect.At(0).GetLocale() );
		
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}
	
	return error_result;
	
} // BE_XPath


FMX_PROC(errcode) BE_XPathAll ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& data_vect, Data& results )
{
	errcode error_result = kNoError;
	TextAutoPtr txt;
	
	try {
		int numParams = data_vect.Size();
		StringAutoPtr xml = ParameterAsUTF8String ( data_vect, 0 );
		StringAutoPtr xpath = ParameterAsUTF8String ( data_vect, 1 );
		StringAutoPtr nsList( new string);
		if (numParams > 2)
			nsList = ParameterAsUTF8String ( data_vect, 2 );
		
		
		results.SetAsText(*ApplyXPathAll (xml, xpath, nsList), data_vect.At(0).GetLocale() );
		
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}
	
	return error_result;
	
} // BE_XPathAll



#pragma mark -
#pragma mark Other / Ungrouped
#pragma mark -


/*
 BE_ExtractScriptVariables implements are somewhat imperfect heuristic for finding
 script variables within chunks of filemaker calculation
 
 try to stip out unwanted text such as strings and comments and then, when a $ is
 found, attempt to guess the where the variable name ends
 */

FMX_PROC(errcode) BE_ExtractScriptVariables ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& data_vect, Data& results)
{
	errcode error_result = kNoError;
	
	try {
		
		BEWStringVector variables;
		WStringAutoPtr calculation = ParameterAsWideString ( data_vect, 0 );

		wstring search_for = L"$/\""; // variables, comments and strings (including escaped strings)
		size_t found = calculation->find_first_of ( search_for );

		while ( found != wstring::npos )
		{
			size_t end = 0;
			size_t search_from = found + 1;
									
			switch ( calculation->at ( found ) ) {
				case L'$': // variables
				{
					/*
					 find the end of the variable
						+ - * / ^ & = ≠ < > ≤ ≥ ( , ; ) [ ] " :: $ }
					 unicode escapes are required on Windows
					 */
					
					end = calculation->find_first_of ( L" ;+-=*/&^<>\t\r[]()\u2260\u2264\u2265,", search_from );
					if ( end == wstring::npos ) {
						end = calculation->size();
					}

					// add the variable to the list
					wstring wanted = calculation->substr ( found, end - found );
					variables.PushBack ( wanted );
					search_from = end + 1;
				}
				break;
					
				case L'/': // comments
					switch ( calculation->at ( search_from ) ) {
						case L'/':
							end = calculation->find ( L"\r", search_from );
							search_from = end + 1;
							break;
							
						case L'*':
							end = calculation->find ( L"*/", search_from );
							search_from = end + 2;
							break;
							
						default:
							break;
					}
					break;
					
				case L'\"': // escaped strings
					end = calculation->find ( L"\"", search_from );
					while ( (end != string::npos) && (calculation->at ( end - 1 ) == L'\\') ) {
						end = calculation->find ( L"\"", end + 1 );
					}
					search_from = end + 1;
					break;
					
//				default:
			}
			
			// this is not on an eternal quest
			if ( (end != string::npos) && (search_from < calculation->size()) ) { 
				found = calculation->find_first_of ( search_for, search_from );
			} else {
				found = string::npos;
			}
		}
		
		results.SetAsText( *(variables.AsValueList()), data_vect.At(0).GetLocale() );
		
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}
	
	return error_result;
	
} // BE_ExtractScriptVariables



FMX_PROC(errcode) BE_ExecuteShellCommand ( short /* funcId */, const ExprEnv& /* environment */, const DataVect& data_vect, Data& results)
{
	errcode error_result = kNoError;
	
	try {
		
		StringAutoPtr command = ParameterAsUTF8String ( data_vect, 0 );
				
		FILE * command_result = POPEN ( command->c_str(), "r" );
		
		if ( command_result ) {

			StringAutoPtr response ( new string ( ) );
			char reply[PATH_MAX];

			while ( fgets ( reply, PATH_MAX, command_result ) != NULL ) {
				response->append ( reply );
			}		
			
			error_result = PCLOSE ( command_result );
			
			SetUTF8Result ( response, results );

		} else {
			error_result = errno;
		}
		
		
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}
	
	return error_result;
	
} // BE_ExecuteShellCommand


/*
 a wrapper for the FileMaker SQL calls FileMaker_Tables and FileMaker_Fields
 
 under FileMaker 11 the functions return
 
 FileMaker_Tables - returns a list of TOs with associated information
	table occurance name
	table occurance id
	table name
	file name
	schema modification count 

 FileMaker_Fields - returns a list of fields...
	table occurance name
	name
	type
	id
	class
	repitions
	modification count 

 Note: 
	For FileMaker versions 8.5~10 a subset of this information is returned. 
	The functions do not exist in versions 7 & 8 and an error is returned.
 
 */

FMX_PROC(errcode) BE_FileMaker_TablesOrFields ( short funcId, const ExprEnv& environment, const DataVect& parameters, Data& reply )
{	
	fmx::errcode error_result = kNoError;
	
	fmx::TextAutoPtr expression;

	try {

		if ( funcId == kBE_FileMaker_Tables ) {
			expression->Assign ( "SELECT * FROM FileMaker_Tables" );
		} else {
			expression->Assign ( "SELECT * FROM FileMaker_Fields" );
		}
		
		// the original api best suits the purpose
		error_result = environment.ExecuteSQL ( *expression, reply, '\t', '\n' );
		
		
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}	
	
	return error_result;
	
} // BE_FileMaker_TablesOrFields


// open the supplied url in the user's default browser

FMX_PROC(errcode) BE_OpenURL ( short funcId, const ExprEnv& environment, const DataVect& parameters, Data& reply )
{	
	fmx::errcode error_result = kNoError;
	
	fmx::TextAutoPtr expression;
	
	try {
		
		WStringAutoPtr url = ParameterAsWideString ( parameters, 0 );
		bool succeeded = OpenURL ( url );

		SetNumericResult ( succeeded, reply );
		
	} catch ( bad_alloc e ) {
		error_result = kLowMemoryError;
	} catch ( exception e ) {
		error_result = kErrorUnknown;
	}	
	
	return error_result;
	
} // BE_OpenURL



