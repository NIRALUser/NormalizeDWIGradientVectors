#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkVectorImage.h>
#include <cstring>
#include <itkMatrix.h>
#include <itkMetaDataObject.h>

#define NORMALIZEDWIGRADIENTVECTORS_VERSION "1.0.0"

//To check the image voxel type
void GetImageType( const char* fileName ,
                   itk::ImageIOBase::IOPixelType &pixelType ,
                   itk::ImageIOBase::IOComponentType &componentType
                 )
{
   typedef itk::Image< unsigned char , 3 > ImageType ;
   itk::ImageFileReader< ImageType >::Pointer imageReader ;
   imageReader = itk::ImageFileReader< ImageType >::New() ;
   imageReader->SetFileName( fileName ) ;
   imageReader->UpdateOutputInformation() ;
   pixelType = imageReader->GetImageIO()->GetPixelType() ;
   componentType = imageReader->GetImageIO()->GetComponentType() ;
}

//Transform the gradient vectors
int NormalizeGradients( itk::MetaDataDictionary &dic )
{
   typedef itk::MetaDataObject< std::string > MetaDataStringType ;
   itk::MetaDataDictionary::ConstIterator itr = dic.Begin() ;
   itk::MetaDataDictionary::ConstIterator end = dic.End() ;
   int dtmri = 0 ;
   while( itr != end )
   {
      itk::MetaDataObjectBase::Pointer entry = itr->second ;
      MetaDataStringType::Pointer entryvalue
            = dynamic_cast<MetaDataStringType* >( entry.GetPointer() ) ;
      if( entryvalue )
      {
         //get the gradient directions
         int pos = itr->first.find( "DWMRI_gradient" ) ;
         if( pos != -1 )
         {
            dtmri = true ;
            std::string tagvalue = entryvalue->GetMetaDataObjectValue() ;
            itk::Vector< double , 3 > vec ;
            itk::Vector< double , 3 > transformedVector ;
            std::istringstream iss( tagvalue ) ;
            iss >> vec[ 0 ] >> vec[ 1 ] >> vec[ 2 ] ;//we copy the metavalue in an itk::vector
            if( iss.fail() )
            {
               iss.str( tagvalue ) ;
               iss.clear() ;
               std::string trash ;
               iss >> vec[ 0 ] >> trash >> vec[ 1 ] >> trash >> vec[ 2 ] ;//in case the separator between the values is something else than spaces
               if( iss.fail() )//problem reading the gradient values
                  {
                     std::cerr << "Error reading a DWMRI gradient value" << std::endl ;
                     return 1 ;
                  }
            }
            //gradient null
            if( vec.GetNorm() > .00001 )
            {
               vec /= vec.GetNorm() ;
            }
            std::ostringstream oss ;
            //write the new gradient values (after transformation) in the metadatadictionary
            oss << vec[ 0 ] << "  " <<
                   vec[ 1 ] << "  " <<
                   vec[ 2 ] << std::ends ;
            entryvalue->SetMetaDataObjectValue( oss.str() ) ;
         }
      }
      ++itr ;
   }
   return dtmri ;
}

int CheckIfDWMRI( itk::MetaDataDictionary &dic )
{
   typedef itk::MetaDataObject< std::string > MetaDataStringType ;
   itk::MetaDataDictionary::ConstIterator itr = dic.Begin() ;
   itk::MetaDataDictionary::ConstIterator end = dic.End() ;
   while( itr != end )
   {
      itk::MetaDataObjectBase::Pointer entry = itr->second ;
      MetaDataStringType::Pointer entryvalue
            = dynamic_cast<MetaDataStringType* >( entry.GetPointer() ) ;
      if( entryvalue )
      {
         //get the gradient directions
         int pos = itr->first.find( "modality" ) ;
         if( pos != -1 )
         {
            std::string tagvalue = entryvalue->GetMetaDataObjectValue() ;
            if( !tagvalue.compare( "DWMRI" ) )
            {
               return 1 ;
            }
         }
      }
      ++itr ;
   }
   return 0 ;
}

template< class PixelType > int Normalize( char* input , char* output )
{
   typedef itk::VectorImage< PixelType , 3 > ImageType ;
   typedef itk::ImageFileReader< ImageType > ReaderType ;
   typedef itk::ImageFileWriter< ImageType > WriterType ;
   itk::MetaDataDictionary dic ;
   typename ReaderType::Pointer reader = ReaderType::New() ;
   reader->SetFileName( input ) ;
   try
   {
      //open image file
      reader->Update() ;
      //Save metadata dictionary
      dic = reader->GetOutput()->GetMetaDataDictionary() ;
   }
   catch( itk::ExceptionObject exception )
   {
      std::cerr << exception << std::endl ;
      return 1 ;
   }
   if( !CheckIfDWMRI( dic ) )
   {
      std::cerr << "This image is not a DWMRI" << std::endl ;
      return 1;
   }
   if( !NormalizeGradients( dic ) )
   {
      std::cerr << "Error while reading the gradients" << std::endl ;
      return 1 ;
   }
   reader->GetOutput()->SetMetaDataDictionary( dic ) ;
   typename WriterType::Pointer writer = WriterType::New() ;
   writer->SetInput( reader->GetOutput() ) ;
   writer->SetFileName( output ) ;
   writer->UseCompressionOn() ;
   try
   {
      writer->Update() ;
   }
   catch( itk::ExceptionObject exception )
   {
      std::cerr << exception << std::endl ;
      return 1 ;
   }
   return 0 ;
}



int main( int argc , char* argv[] )
{
   if( argc == 2 && !strcmp(argv[ 1 ],"--version") )
   {
     std::cout << "NormalizeDWIGradientVectors version: " << NORMALIZEDWIGRADIENTVECTORS_VERSION << std::endl ;
     return 0 ;
   }
   if( argc != 3 )
   {
      std::cerr << argv[ 0 ] << " inputDWI outputDWI" << std::endl ;
      std::cerr << argv[ 0 ] << " --version -> Prints version " << std::endl ;
      return 1 ;
   }
   itk::ImageIOBase::IOPixelType pixelType ;
   itk::ImageIOBase::IOComponentType componentType ;
   GetImageType( argv[ 1 ] , pixelType , componentType ) ;
   //templated over the input image voxel type
   switch( componentType )
   {
      case itk::ImageIOBase::UCHAR:
         return Normalize< unsigned char >( argv[ 1 ] , argv[ 2 ] ) ;
         break ;
      case itk::ImageIOBase::CHAR:
         return Normalize< char >( argv[ 1 ] , argv[ 2 ] ) ;
         break ;
      case itk::ImageIOBase::USHORT:
         return Normalize< unsigned short >( argv[ 1 ] , argv[ 2 ] ) ;
         break ;
      case itk::ImageIOBase::SHORT:
         return Normalize< short >( argv[ 1 ] , argv[ 2 ] ) ;
         break ;
      case itk::ImageIOBase::UINT:
         return Normalize< unsigned int >( argv[ 1 ] , argv[ 2 ] ) ;
         break ;
      case itk::ImageIOBase::INT:
         return Normalize< int >( argv[ 1 ] , argv[ 2 ] ) ;
         break ;
      case itk::ImageIOBase::ULONG:
         return Normalize< unsigned long >( argv[ 1 ] , argv[ 2 ] ) ;
         break ;
      case itk::ImageIOBase::LONG:
         return Normalize< long >( argv[ 1 ] , argv[ 2 ] ) ;
         break ;
      case itk::ImageIOBase::FLOAT:
         return Normalize< float >( argv[ 1 ] , argv[ 2 ] ) ;
         break ;
      case itk::ImageIOBase::DOUBLE:
         return Normalize< double >( argv[ 1 ] , argv[ 2 ] ) ;
         break ;
      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
      default:
         std::cerr << "unknown component type" << std::endl ;
         break ;
   }
   return 1 ;
}
