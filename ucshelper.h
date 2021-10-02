#ifndef DFUCSHELPER_H_
#define DFUCSHELPER_H_
#include <windows.h>
// Create a utf8 string from a potentially UCS-2 string
// This is used to provide a buffer into which a UCS-2/UTF-16 string
// can be written by an API. 
// Call set with the size. Then call getUCBuffer to get the buffer.
// After the buffer has been filled, call utf8String to get the string
// as utf-8

class  UCToUTF8 {

   WCHAR *mValue; 
   char *mUtfValue; 
   int mLen;
public: 
   UCToUTF8() : mValue(0), mUtfValue(0), mLen(0) {} 
   UCToUTF8(WCHAR *str): mValue(0), mUtfValue(0), mLen(0)  {
      size_t len = wcslen(str);
      set(len);
      memcpy(mValue, str, len * sizeof(WCHAR));
   }
   ~UCToUTF8() {
      set(0); 
   } 
   // returns a pointer to the allocated buffer 
   WCHAR *getUCBuffer() { return mValue; } 
   // allocate the buffer with size, this will free any old buffer 
   // returns false on memalloc error
   bool set(int size); 
   // return the string as a utf-8 value 
   // returns 0 on memalloc failure 
   char *utf8String(); 
   // return the buffer length in chars
   int bufLength() { return mLen; } 
   void release() {delete this;}
   int bufLengthBytes() {return mLen * sizeof(WCHAR); }
};
// create a UCS-2 string from a utf8 string
// Given a string in UTF-8 will convert it to wide char.
// Call set with the string,
// then call ucString to get the wide string.
// bufLength returns the length in characters

class  Utf8ToUC {
   int mLen;
   WCHAR *mValue;
public:
   Utf8ToUC() : mLen(0), mValue(0) {}
   ~Utf8ToUC() { set(0); }
   WCHAR *ucString() { return mValue; }
   // length in wide chars
   int bufLength() { return mLen; }
   // returns false on memalloc failure
   bool set(const char *utf8str);
   void release() { delete this; }

   int bufLengthBytes() {
      return mLen * sizeof(WCHAR);
   }
};

// destbuflen is in bytes
void convertUCToUtf8(WCHAR *source, char *dest, int destbuflen);
// destbuflen is in characters
void convertUtf8ToUC(char *source, WCHAR *dest, int destbuflen);

#endif