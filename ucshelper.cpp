#include <windows.h>
#include "ucshelper.h"

bool UCToUTF8::set(int size) { 
   if(mValue) 
      free(mValue);
   mLen=size;

   if(mUtfValue) 
      free (mUtfValue); 
   mUtfValue=0; 
   if(size) {
      mValue = (WCHAR*) malloc(sizeof(WCHAR) * (size + 1));
      if(!mValue)
         return false; 
      mValue[size]=0;
   }
   return true;
} 

char *UCToUTF8::utf8String() {
   if(!mUtfValue) {
      int newlen; 
      newlen=WideCharToMultiByte(CP_UTF8,0,mValue,(int)wcslen(mValue)+1,0,0,0,0);
      if(!(mUtfValue=(char *)malloc(newlen)))
         return 0;
      memset(mUtfValue, 0, newlen);
      WideCharToMultiByte(CP_UTF8,0,mValue,mLen,mUtfValue,newlen,0,0);
   }
   return mUtfValue;
}
bool Utf8ToUC::set(const char *utf8str) {
   if(mValue)
      free(mValue);
   if(utf8str)
      mLen=MultiByteToWideChar(CP_UTF8, 0, utf8str,-1,0 ,0);
   else 
      mLen=0;

   if(mLen) { 
      if (!(mValue = (WCHAR*)malloc(sizeof(WCHAR) * mLen)))
         return false; 
      memset(mValue, 0, sizeof(WCHAR) * mLen);
      MultiByteToWideChar(CP_UTF8, 0, utf8str,-1,mValue ,mLen);
   } 
   else 
      mValue=0; 
   return true;
}
void convertUCToUtf8(WCHAR *source, char *dest, int destbuflen)
{
   int len=WideCharToMultiByte(CP_UTF8,0,source,-1,dest,destbuflen,0,0);
   if(len == destbuflen) 
      dest[destbuflen-1]=0; 
}
void convertUtf8ToUC(char *source, WCHAR *dest, int destbuflen){
   MultiByteToWideChar(CP_UTF8, 0, source,-1, dest, destbuflen);
}
