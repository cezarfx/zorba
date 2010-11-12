/*
 * Copyright 2006-2008 The FLWOR Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sstream>
#include <zorba/zorba.h>
#include <Magick++.h>
#include <zorba/base64.h>
#include <zorba/empty_sequence.h>
#include <zorba/singleton_item_sequence.h>
#include "image_function.h"
#include "image_module.h"

namespace zorba {  namespace imagemodule {

ImageFunction::ImageFunction(const ImageModule* aModule)
        : theModule(aModule)
{
}

ImageFunction::~ImageFunction()
{
}

String
ImageFunction::getURI() const
{
        return theModule->getURI();
}
void
ImageFunction::throwError(
        const std::string aErrorMessage,
        const XQUERY_ERROR& aErrorType)
{
  throw zorba::ExternalFunctionData::createZorbaException(aErrorType,
              aErrorMessage.c_str(), __FILE__, __LINE__);
}

void
ImageFunction::throwImageError(const ImageModule* aModule, const char *aMessage) {
  std::stringstream lErrorMessage;
  // constuct error QName
  String lNamespace = "http://www.zorba-xquery.com/modules/image/error";
  String lLocalname = "err:IM001";
  Item lQName = aModule->getItemFactory()->createQName(lNamespace, lLocalname);


  // if we have zero length image, then tell the user so
  if (std::string(aMessage).find("zero-length") != std::string::npos) {
    lErrorMessage << "The passed xs:base64Binary seems to be empty.";
    error(lQName, lErrorMessage.str());
  } else {
    lErrorMessage << "Error while processing xs:base64Binary. Possibly not a valid image type.";
    error(lQName, lErrorMessage.str());
  }
}


void
ImageFunction::checkIfItemIsNull(Item& aItem) {
  if (aItem.isNull()) {
    throw ExternalFunctionData::createZorbaException(XPST0083, "Error while building the base64binary item. ", __FILE__, __LINE__);
  }
}
String
ImageFunction::getOneStringArg(
    const StatelessExternalFunction::Arguments_t& aArgs,
    int aPos)
{
  Item lItem;
  if (!aArgs[aPos]->next(lItem)) {
    std::stringstream lErrorMessage;
    lErrorMessage << "An empty-sequence is not allowed as "
                  << aPos << ". parameter.";
    throwError(lErrorMessage.str(), XPTY0004);
  }
  zorba::String lTmpString = lItem.getStringValue();
  if (aArgs[aPos]->next(lItem)) {
    std::stringstream lErrorMessage;
    lErrorMessage << "A sequence of more then one item is not allowed as "
                  << aPos << ". parameter.";
    throwError(lErrorMessage.str(), XPTY0004);
  }
  return lTmpString;
}

bool
ImageFunction::getOneBoolArg(
    const StatelessExternalFunction::Arguments_t& aArgs,
    int aPos)
{
  Item lItem;
  if (!aArgs[aPos]->next(lItem)) {
    std::stringstream lErrorMessage;
    lErrorMessage << "An empty-sequence is not allowed as "
                  << aPos << ". parameter.";
    throwError(lErrorMessage.str(), XPTY0004);
  }
  bool lTmpBool = lItem.getBooleanValue();
  if (aArgs[aPos]->next(lItem)) {
    std::stringstream lErrorMessage;
    lErrorMessage << "A sequence of more then one item is not allowed as "
                  << aPos << ". parameter.";
    throwError(lErrorMessage.str(), XPTY0004);
  }
  return lTmpBool;
}

void
ImageFunction::getOneColorArg(
     const StatelessExternalFunction::Arguments_t& aArgs,
     int aPos,
     Magick::ColorRGB& aColor)
{
  Item lItem;
  if (!aArgs[aPos]->next(lItem)) {
    aColor = Magick::ColorRGB(0.0,0.0,0.0);
    return;
  }
  zorba::String lTmpString = lItem.getStringValue();
  if (aArgs[aPos]->next(lItem)) {
    std::stringstream lErrorMessage;
    lErrorMessage << "A sequence of more then one item is not allowed as "
                  << aPos << ". parameter.";
    throwError(lErrorMessage.str(), XPTY0004);
  }
  getColorFromString(lTmpString, aColor);
}



void 
ImageFunction::getColorFromString(const String aColorString,
                                  Magick::ColorRGB& aColor)
{
  int lRed = 0;
  int lGreen = 0;
  int lBlue = 0;
  sscanf(aColorString.substring(1,2).c_str(), "%x", &lRed);
  sscanf(aColorString.substring(3,2).c_str(), "%x", &lGreen);
  sscanf(aColorString.substring(5,2).c_str(), "%x", &lBlue);
  aColor = Magick::ColorRGB((double)lRed/(double)255.0, (double)lGreen/(double)255.0, (double)lBlue/(double)255.0);

}

int
ImageFunction::getOneIntArg(
    const StatelessExternalFunction::Arguments_t& aArgs,
    int aPos)
{
  Item lItem;
  if (!aArgs[aPos]->next(lItem)) {
    std::stringstream lErrorMessage;
    lErrorMessage << "An empty-sequence is not allowed as "
                  << (aPos + 1)  << ". parameter.";
    throwError(lErrorMessage.str(), XPTY0004);
  }
  int lTmpInt = (int) lItem.getIntValue();
  if (aArgs[aPos]->next(lItem)) {
    std::stringstream lErrorMessage;
    lErrorMessage << "A sequence of more then one item is not allowed as "
                  << (aPos + 1)  << ". parameter.";
    throwError(lErrorMessage.str(), XPTY0004);
  }
  return lTmpInt;

}

unsigned int
ImageFunction::getOneUnsignedIntArg(const StatelessExternalFunction::Arguments_t& aArgs,
                                    int aPos)
{
  Item lItem;
  if (!aArgs[aPos]->next(lItem)) {
    std::stringstream lErrorMessage;
    lErrorMessage << "An empty-sequence is not allowed as "
                  << (aPos + 1) << ". parameter.";
    throwError(lErrorMessage.str(), XPTY0004);
  }
  unsigned int lTmpInt = lItem.getUnsignedIntValue();
  if (aArgs[aPos]->next(lItem)) {
    std::stringstream lErrorMessage;
    lErrorMessage << "A sequence of more then one item is not allowed as "
                  << (aPos + 1) << ". parameter.";
    throwError(lErrorMessage.str(), XPTY0004);
  }
  return lTmpInt;

}

double
ImageFunction::getOneDoubleArg(
    const StatelessExternalFunction::Arguments_t& aArgs,
    int aPos)
{
  Item lItem;
  if (!aArgs[aPos]->next(lItem)) {
    std::stringstream lErrorMessage;
    lErrorMessage << "An empty-sequence is not allowed as "
                  << (aPos + 1)  << ". parameter.";
    throwError(lErrorMessage.str(), XPTY0004);
  }
  double lTmpDouble =  lItem.getDoubleValue();
  if (aArgs[aPos]->next(lItem)) {
    std::stringstream lErrorMessage;
    lErrorMessage << "A sequence of more then one item is not allowed as "
                  << (aPos + 1) << ". parameter.";
    throwError(lErrorMessage.str(), XPTY0004);
  }
  return lTmpDouble;
}


String
ImageFunction::getEncodedStringFromBlob(Magick::Blob& aBlob) {

    std::string lStringOfBlobContent((char *)aBlob.data(), aBlob.length());
    String lZorbaStringOfBlobContent(lStringOfBlobContent);

    return zorba::encoding::Base64::encode(lZorbaStringOfBlobContent);
}

String
ImageFunction::getEncodedStringFromImage(const ImageModule* aModule, Magick::Image& aImage) {
  Magick::Blob lBlob;
  try {
    aImage.write(&lBlob);
  } catch (Magick::Exception& error) {
    throwImageError(aModule, error.what());
  }
  return getEncodedStringFromBlob(lBlob);
}





void
ImageFunction::getOneImageArg(const ImageModule* aModule,
                              const StatelessExternalFunction::Arguments_t& aArgs,
                              int aPos,
                              Magick::Image& aImage)
{
  String lData;
  lData = getOneStringArg(aArgs, aPos);
  getImageFromString(aModule, lData, aImage);
}

void
ImageFunction::getOneOrMoreImageArg( const ImageModule* aModule,
                                     const StatelessExternalFunction::Arguments_t& aArgs,
                                     int aPos,
                                     std::list<Magick::Image>& aImages,
                                     const unsigned int aDelay,
                                     const unsigned int aIterations)

{
  Item lItem;
  // make sure there is at least one item at the position
  if (!aArgs[aPos]->next(lItem)) {
    throwError("An empty sequence is not allowed as first parameter", XPTY0004);
  }

  Magick::Image lFirstImage;
  ImageFunction::getImageFromString(aModule, lItem.getStringValue(), lFirstImage);
  lFirstImage.animationDelay(aDelay);
  lFirstImage.animationIterations(aIterations);
  lFirstImage.gifDisposeMethod(3);
  aImages.push_back(lFirstImage);
  Magick::Image lTempImage;
  while (aArgs[aPos]->next(lItem)) {
    getImageFromString(aModule, lItem.getStringValue(), lTempImage);
    aImages.push_back(lTempImage);
  }

}

void
ImageFunction::getImageFromString(const ImageModule* aModule, const String aString, Magick::Image& aImage) {

  String lDecodedContent = zorba::encoding::Base64::decode(aString);
  Magick::Blob lBlob(lDecodedContent.c_str(), lDecodedContent.bytes());

  try {
    aImage.read(lBlob);

  } catch (Magick::Exception &error)   {
      throwImageError(aModule, error.what());
  }
}


bool
ImageFunction::getAntiAliasingArg(
    const StatelessExternalFunction::Arguments_t& aArgs,
    int aPos)
{
  Item lItem;
  if (!aArgs[aPos]->next(lItem)) {
    return false;
  }
  bool lTmpBool = lItem.getBooleanValue();
  if (aArgs[aPos]->next(lItem)) {
    std::stringstream lErrorMessage;
    lErrorMessage << "A sequence of more then one item is not allowed as "
                  << (aPos + 1) << ". parameter.";
    throwError(lErrorMessage.str(), XPTY0004);
  }
  return lTmpBool;
}

double
ImageFunction::getStrokeWidthArg(const StatelessExternalFunction::Arguments_t& aArgs,
                                 int aPos)
{
  Item lItem;
  if (!aArgs[aPos]->next(lItem)) {
    return 1;
  }
  double lTmpDouble = lItem.getDoubleValue();
  if (aArgs[aPos]->next(lItem)) {
    std::stringstream lErrorMessage;
    lErrorMessage << "A sequence of more then one item is not allowed as "
                  << (aPos + 1) << ". parameter.";
    throwError(lErrorMessage.str(), XPTY0004);
  }
  return lTmpDouble;
}



} // imagemodule 
} // zorba


