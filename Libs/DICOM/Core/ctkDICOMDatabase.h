/*=========================================================================

  Library:   CTK

  Copyright (c) 2010

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=========================================================================*/

#ifndef __ctkDICOMDatabase_h
#define __ctkDICOMDatabase_h

// Qt includes
#include <QObject>
#include <QStringList>
#include <QSqlDatabase>

#include "ctkDICOMDataset.h"
#include "ctkDICOMCoreExport.h"

class ctkDICOMDatabasePrivate;
class DcmDataset;
class ctkDICOMAbstractThumbnailGenerator;

/// \ingroup DICOM_Core
///
/// Class handling a database of DICOM objects. So far, an underlying
/// SQLITE database is used for that. Usually, added DICOM objects are also
/// stored within the file system.
/// The SQLITE database file can be specified by the user. SQLITE (and this
/// class) also support a special in memory mode, where no database file is created
/// but the database is completely kept in memory (and after exiting the program,
/// vanishes). If in "memory mode", the objects are not written to disk,
/// otherwise they are stored in a subdirectory of the SQLITE database file
/// directory called "dicom". Inside, a folder structure created which contains
/// a directoy for each study, containing a directory for each series, containing
/// a file for each object. The corresponding UIDs are used as filenames.
/// Thumbnais for each image can be created; if so, they are stored in a directory
/// parallel to "dicom" directory called "thumbs".
class CTK_DICOM_CORE_EXPORT ctkDICOMDatabase : public QObject
{

  Q_OBJECT
  Q_PROPERTY(bool isOpen READ isOpen)
  Q_PROPERTY(QString lastError READ lastError)
  Q_PROPERTY(QString databaseFilename READ databaseFilename)

public:
  explicit ctkDICOMDatabase(QObject *parent = 0);
  explicit ctkDICOMDatabase(QString databaseFile);
  virtual ~ctkDICOMDatabase();

  const QSqlDatabase& database() const;
  const QString lastError() const;
  const QString databaseFilename() const;

  ///
  /// Returns the absolute path of the database directory
  /// (where the database file resides in) in OS-prefered path format.
  /// @return Absolute path to database directory
  const QString databaseDirectory() const;

  ///
  /// Should be checked after trying to open the database
  /// @Returns true if database is open
  bool isOpen() const;

  ///
  /// Returns whether the database only resides in memory, i.e. the
  /// SQLITE DB is not written to stored to disk and DICOM objects are not
  /// stored to the file system.
  /// @return True if in memory mode, false otherwise.
  bool isInMemory() const;

  ///
  /// set thumbnail generator object
  void setThumbnailGenerator(ctkDICOMAbstractThumbnailGenerator* generator);
  ///
  /// get thumbnail genrator object
  ctkDICOMAbstractThumbnailGenerator* thumbnailGenerator();

  ///
  /// open the SQLite database in @param databaseFile . If the file does not
  /// exist, a new database is created and initialized with the
  /// default schema
  ///
  /// @param databaseFile The file to store the SQLITE database should be
  ///        stored to. If specified with ":memory:", the database is not
  ///        written to disk at all but instead only kept in memory (and
  ///        thus expires after destruction of this object).
  /// @param connectionName The database connection name.
  Q_INVOKABLE virtual void openDatabase(const QString databaseFile,
                                        const QString& connectionName = "DICOM-DB" );

  ///
  /// close the database. It must not be used afterwards.
  Q_INVOKABLE void closeDatabase();
  ///
  /// delete all data and reinitialize the database.
  Q_INVOKABLE bool initializeDatabase(const char* schemaFile = ":/dicom/dicom-schema.sql");

  ///
  /// \brief database accessors
  Q_INVOKABLE QStringList patients ();
  Q_INVOKABLE QStringList studiesForPatient (const QString patientUID);
  Q_INVOKABLE QStringList seriesForStudy (const QString studyUID);
  Q_INVOKABLE QStringList filesForSeries (const QString seriesUID);
  Q_INVOKABLE QString fileForInstance (const QString sopInstanceUID);

  ///
  /// \brief load the header from a file and allow access to elements
  /// @param sopInstanceUID A string with the uid for a given instance
  ///                       (corresponding file will be found via database)
  /// @param fileName Full path to a dicom file to load.
  /// @param key A group,element tag in zero-filled hex
  Q_INVOKABLE void loadInstanceHeader (const QString sopInstanceUID);
  Q_INVOKABLE void loadFileHeader (const QString fileName);
  Q_INVOKABLE QStringList headerKeys ();
  Q_INVOKABLE QString headerValue (const QString key);

  /// Insert into the database if not already exsting.
  /// @param dataset The dataset to store into the database. Usually, this is
  ///                is a complete DICOM object, like a complete image. However
  ///                the database also inserts partial objects, like studyl
  ///                information to the database, even if no image data is
  ///                contained. This can be helpful to store results from
  ///                querying the PACS for patient/study/series or image
  ///                information, where a full hierarchy is only constructed
  ///                after some queries.
  /// @param storeFile If store file is set (default), then the dataset will
  ///                  be stored to disk. Note that in case of a memory-only
  ///                  database, this flag is ignored. Usually, this flag
  ///                  does only make sense if a full object is received.
  /// @param @generateThumbnail If true, a thumbnail is generated.
  ///
  Q_INVOKABLE void insert( const ctkDICOMDataset& ctkDataset, bool storeFile, bool generateThumbnail);
  void insert ( DcmDataset *dataset, bool storeFile = true, bool generateThumbnail = true);
  Q_INVOKABLE void insert ( const QString& filePath, bool storeFile = true, bool generateThumbnail = true, bool createHierarchy = true, const QString& destinationDirectoryName = QString() );
  
  /// Check if file is already in database and up-to-date
  bool fileExistsAndUpToDate(const QString& filePath);

  /// remove the series from the database, including images and
  /// thumbnails
  Q_INVOKABLE bool removeSeries(const QString& seriesInstanceUID);
  Q_INVOKABLE bool removeStudy(const QString& studyInstanceUID);
  Q_INVOKABLE bool removePatient(const QString& patientID);
  bool cleanup();

  ///
  /// \brief access element values for given instance
  /// @param sopInstanceUID A string with the uid for a given instance
  ///                       (corresponding file will be found via database)
  /// @param fileName Full path to a dicom file to load.
  /// @param key A group,element tag in zero-filled hex
  /// @param group The group portion of the tag as an integer
  /// @param element The element portion of the tag as an integer
  /// @Returns empty string is element is missing
  Q_INVOKABLE QString instanceValue (const QString sopInstanceUID, const QString tag);
  Q_INVOKABLE QString instanceValue (const QString sopInstanceUID, const unsigned short group, const unsigned short element);
  Q_INVOKABLE QString fileValue (const QString fileName, const QString tag);
  Q_INVOKABLE QString fileValue (const QString fileName, const unsigned short group, const unsigned short element);
  bool tagToGroupElement (const QString tag, unsigned short& group, unsigned short& element);


Q_SIGNALS:
  void databaseChanged();

protected:
  QScopedPointer<ctkDICOMDatabasePrivate> d_ptr;



private:
  Q_DECLARE_PRIVATE(ctkDICOMDatabase);
  Q_DISABLE_COPY(ctkDICOMDatabase);
};

#endif

