/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

//  CCdInfo   -  Information about media type of an inserted cd
//  CCdIoSupport -  Wrapper class for libcdio with the interface of CIoSupport
//     and detecting the filesystem on the Disc.
//
// by Bobbin007 in 2003
//  CD-Text support by Mog - Oct 2004

#include "threads/CriticalSection.h"

#include <map>
#include <memory>
#include <string>

typedef struct _CdIo CdIo_t; 

namespace MEDIA_DETECT
{

#define STRONG "__________________________________\n"
//#define NORMAL ""

#define FS_NO_DATA 0 /* audio only */
#define FS_HIGH_SIERRA 1
#define FS_ISO_9660 2
#define FS_INTERACTIVE 3
#define FS_HFS 4
#define FS_UFS 5
#define FS_EXT2 6
#define FS_ISO_HFS 7 /* both hfs & isofs filesystem */
#define FS_ISO_9660_INTERACTIVE 8 /* both CD-RTOS and isofs filesystem */
#define FS_3DO 9
#define FS_UDF 11
#define FS_ISO_UDF 12
#define FS_UNKNOWN 15
#define FS_MASK 15

#define XA 16
#define MULTISESSION 32
#define PHOTO_CD 64
#define HIDDEN_TRACK 128
#define CDTV 256
#define BOOTABLE 512
#define VIDEOCDI 1024
#define ROCKRIDGE 2048
#define JOLIET 4096
#define CVD 8192 /* Choiji Video CD */

#define IS_ISOFS 0
#define IS_CD_I 1
#define IS_CDTV 2
#define IS_CD_RTOS 3
#define IS_HS 4
#define IS_BRIDGE 5
#define IS_XA 6
#define IS_PHOTO_CD 7
#define IS_EXT2 8
#define IS_UFS 9
#define IS_BOOTABLE 10
#define IS_VIDEO_CD 11 /* Video CD */
#define IS_CVD 12 /* Chinese Video CD - slightly incompatible with SVCD */
#define IS_UDF 14

typedef struct signature
{
  unsigned int buf_num;
  unsigned int offset;
  const char* sig_str;
  const char* description;
} signature_t;

/*! Enumeration of CD-TEXT text fields. */
enum class CdTextField
{
  TITLE = 0, /**< title of album name or track titles */
  PERFORMER = 1, /**< name(s) of the performer(s) */
  SONGWRITER = 2, /**< name(s) of the songwriter(s) */
  COMPOSER = 3, /**< name(s) of the composer(s) */
  MESSAGE = 4, /**< message(s) from content provider or artist, ISO-8859-1 encoded*/
  ARRANGER = 5, /**< name(s) of the arranger(s) */
  ISRC = 6, /**< ISRC code of each track */
  UPC_EAN = 7, /**< upc/european article number of disc, ISO-8859-1 encoded */
  GENRE = 8, /**< genre identification and genre information, ASCII encoded */
  DISCID = 9, /**< disc identification, ASCII encoded (may be non-printable) */
  INVALID = 10 /**< INVALID FIELD*/
};

/**
   * The driver_id_t enumerations may be used to tag a specific driver
   * that is opened or is desired to be opened. Note that this is
   * different than what is available on a given host.
   *
   * Order should not be changed lightly because it breaks the ABI.
   * One is not supposed to iterate over the values, but iterate over the
   * cdio_drivers and cdio_device_drivers arrays.
   *
   * NOTE: IF YOU MODIFY ENUM MAKE SURE INITIALIZATION BETWEEN CDIO.C AGREES.
   *
   */
enum class CdDriver
{
  Unknown, /**< Used as input when we don't care what kind
                         of driver to use. */
  Win32, /**< Microsoft Windows Driver. Includes ASPI and
                         ioctl access. */
  Cdrdao, /**< cdrdao format CD image. This is listed
                         before BIN/CUE, to make the code prefer cdrdao
                         over BIN/CUE when both exist. */
  Bincue, /**< CDRWIN BIN/CUE format CD image. This is
                         listed before NRG, to make the code prefer
                         BIN/CUE over NRG when both exist. */
  Nrg, /**< Nero NRG format CD image. */
  Device /**< Is really a set of the above; should come last */
};

/**
       disc modes. The first combined from MMC-5 6.33.3.13 (Send
       CUESHEET), "DVD Book" from MMC-5 Table 400, page 419.  and
       GNU/Linux /usr/include/linux/cdrom.h and we've added DVD.
    */
enum class CdDiscMode
{
  CD_DA, /**< CD-DA */
  CD_DATA, /**< CD-ROM form 1 */
  CD_XA, /**< CD-ROM XA form2 */
  CD_MIXED, /**< Some combo of above. */
  DVD_ROM, /**< DVD ROM (e.g. movies) */
  DVD_RAM, /**< DVD-RAM */
  DVD_R, /**< DVD-R */
  DVD_RW, /**< DVD-RW */
  HD_DVD_ROM, /**< HD DVD-ROM */
  HD_DVD_RAM, /**< HD DVD-RAM */
  HD_DVD_R, /**< HD DVD-R */
  DVD_PR, /**< DVD+R */
  DVD_PRW, /**< DVD+RW */
  DVD_PRW_DL, /**< DVD+RW DL */
  DVD_PR_DL, /**< DVD+R DL */
  DVD_OTHER, /**< Unknown/unclassified DVD type */
  NO_INFO,
  ERRORMODE,
  CD_I /**< CD-i. */
};

/**
      The following are status codes for completion of a given cdio
      operation. By design 0 is successful completion and -1 is error
      completion. This is compatable with ioctl so those routines that
      call ioctl can just pass the value the get back (cast as this
      enum). Also, by using negative numbers for errors, the
      enumeration values below can be used in places where a positive
      value is expected when things complete successfully. For example,
      get_blocksize returns the blocksize, but on error uses the error
      codes below. So note that this enumeration is often cast to an
      integer.  C seems to tolerate this.
  */
enum class CdDriverReturnCode
{
  OP_SUCCESS = 0, /**< in cases where an int is
                                    returned, like cdio_set_speed,
                                    more the negative return codes are
                                    for errors and the positive ones
                                    for success. */
  OP_ERROR = -1, /**< operation returned an error */
  OP_UNSUPPORTED = -2, /**< returned when a particular driver
                                      doesn't support a particular operation.
                                      For example an image driver which doesn't
                                      really "eject" a CD.
                                   */
  OP_UNINIT = -3, /**< returned when a particular driver
                                      hasn't been initialized or a null
                                      pointer has been passed.
                                   */
  OP_NOT_PERMITTED = -4, /**< Operation not permitted.
                                      For example might be a permission
                                      problem.
                                   */
  OP_BAD_PARAMETER = -5, /**< Bad parameter passed  */
  OP_BAD_POINTER = -6, /**< Bad pointer to memory area  */
  OP_NO_DRIVER = -7, /**< Operation called on a driver
                                      not available on this OS  */
  OP_MMC_SENSE_DATA = -8, /**< MMC operation returned sense data,
                                      but no other error above recorded. */
};


typedef std::map<CdTextField, std::string> xbmc_cdtext_t;

typedef struct TRACKINFO
{
  int nfsInfo; // Information of the Tracks Filesystem
  int nJolietLevel; // Jouliet Level
  int ms_offset; // Multisession Offset
  int isofs_size; // Size of the ISO9660 Filesystem
  int nFrames; // Can be used for cddb query
  int nMins; // minutes playtime part of Track
  int nSecs; // seconds playtime part of Track
  xbmc_cdtext_t cdtext; // CD-Text for this track
} trackinfo;


class CCdInfo
{
public:
  CCdInfo()
  {
    m_bHasCDDBInfo = true;
    m_nLength = m_nFirstTrack = m_nNumTrack = m_nNumAudio = m_nFirstAudio = m_nNumData =
        m_nFirstData = 0;
  }

  trackinfo GetTrackInformation(int nTrack) { return m_ti[nTrack - 1]; }
  xbmc_cdtext_t GetDiscCDTextInformation() { return m_cdtext; }

  bool HasDataTracks() { return (m_nNumData > 0); }
  bool HasAudioTracks() { return (m_nNumAudio > 0); }
  int GetFirstTrack() { return m_nFirstTrack; }
  int GetTrackCount() { return m_nNumTrack; }
  int GetFirstAudioTrack() { return m_nFirstAudio; }
  int GetFirstDataTrack() { return m_nFirstData; }
  int GetDataTrackCount() { return m_nNumData; }
  int GetAudioTrackCount() { return m_nNumAudio; }
  uint32_t GetCddbDiscId() { return m_ulCddbDiscId; }
  int GetDiscLength() { return m_nLength; }
  std::string GetDiscLabel() { return m_strDiscLabel; }

  // CD-ROM with ISO 9660 filesystem
  bool IsIso9660(int nTrack) { return ((m_ti[nTrack - 1].nfsInfo & FS_MASK) == FS_ISO_9660); }
  // CD-ROM with joliet extension
  bool IsJoliet(int nTrack) { return (m_ti[nTrack - 1].nfsInfo & JOLIET) ? false : true; }
  // Joliet extension level
  int GetJolietLevel(int nTrack) { return m_ti[nTrack - 1].nJolietLevel; }
  // ISO filesystem size
  int GetIsoSize(int nTrack) { return m_ti[nTrack - 1].isofs_size; }
  // CD-ROM with rockridge extensions
  bool IsRockridge(int nTrack) { return (m_ti[nTrack - 1].nfsInfo & ROCKRIDGE) ? false : true; }

  // CD-ROM with CD-RTOS and ISO 9660 filesystem
  bool IsIso9660Interactive(int nTrack)
  {
    return ((m_ti[nTrack - 1].nfsInfo & FS_MASK) == FS_ISO_9660_INTERACTIVE);
  }

  // CD-ROM with High Sierra filesystem
  bool IsHighSierra(int nTrack) { return ((m_ti[nTrack - 1].nfsInfo & FS_MASK) == FS_HIGH_SIERRA); }

  // CD-Interactive, with audiotracks > 0 CD-Interactive/Ready
  bool IsCDInteractive(int nTrack)
  {
    return ((m_ti[nTrack - 1].nfsInfo & FS_MASK) == FS_INTERACTIVE);
  }

  // CD-ROM with Macintosh HFS
  bool IsHFS(int nTrack) { return ((m_ti[nTrack - 1].nfsInfo & FS_MASK) == FS_HFS); }

  // CD-ROM with both Macintosh HFS and ISO 9660 filesystem
  bool IsISOHFS(int nTrack) { return ((m_ti[nTrack - 1].nfsInfo & FS_MASK) == FS_ISO_HFS); }

  // CD-ROM with both UDF and ISO 9660 filesystem
  bool IsISOUDF(int nTrack) { return ((m_ti[nTrack - 1].nfsInfo & FS_MASK) == FS_ISO_UDF); }

  // CD-ROM with Unix UFS
  bool IsUFS(int nTrack) { return ((m_ti[nTrack - 1].nfsInfo & FS_MASK) == FS_UFS); }

  // CD-ROM with Linux second extended filesystem
  bool IsEXT2(int nTrack) { return ((m_ti[nTrack - 1].nfsInfo & FS_MASK) == FS_EXT2); }

  // CD-ROM with Panasonic 3DO filesystem
  bool Is3DO(int nTrack) { return ((m_ti[nTrack - 1].nfsInfo & FS_MASK) == FS_3DO); }

  // Mixed Mode CD-ROM
  bool IsMixedMode(int nTrack) { return (m_nFirstData == 1 && m_nNumAudio > 0); }

  // CD-ROM with XA sectors
  bool IsXA(int nTrack) { return (m_ti[nTrack - 1].nfsInfo & XA) ? false : true; }

  // Multisession CD-ROM
  bool IsMultiSession(int nTrack)
  {
    return (m_ti[nTrack - 1].nfsInfo & MULTISESSION) ? false : true;
  }
  // Gets multisession offset
  int GetMultisessionOffset(int nTrack) { return m_ti[nTrack - 1].ms_offset; }

  // Hidden Track on Audio CD
  bool IsHiddenTrack(int nTrack)
  {
    return (m_ti[nTrack - 1].nfsInfo & HIDDEN_TRACK) ? false : true;
  }

  // Photo CD, with audiotracks > 0 Portfolio Photo CD
  bool IsPhotoCd(int nTrack) { return (m_ti[nTrack - 1].nfsInfo & PHOTO_CD) ? false : true; }

  // CD-ROM with Commodore CDTV
  bool IsCdTv(int nTrack) { return (m_ti[nTrack - 1].nfsInfo & CDTV) ? false : true; }

  // CD-Plus/Extra
  bool IsCDExtra(int nTrack) { return (m_nFirstData > 1); }

  // Bootable CD
  bool IsBootable(int nTrack) { return (m_ti[nTrack - 1].nfsInfo & BOOTABLE) ? false : true; }

  // Video CD
  bool IsVideoCd(int nTrack) { return (m_ti[nTrack - 1].nfsInfo & VIDEOCDI && m_nNumAudio == 0); }

  // Chaoji Video CD
  bool IsChaojiVideoCD(int nTrack) { return (m_ti[nTrack - 1].nfsInfo & CVD) ? false : true; }

  // Audio Track
  bool IsAudio(int nTrack) { return ((m_ti[nTrack - 1].nfsInfo & FS_MASK) == FS_NO_DATA); }

  // UDF filesystem
  bool IsUDF(int nTrack) { return ((m_ti[nTrack - 1].nfsInfo & FS_MASK) == FS_UDF); }

  // Has the cd a filesystem that is readable by the xbox
  bool IsValidFs()
  {
    return (IsISOHFS(1) || IsIso9660(1) || IsIso9660Interactive(1) || IsISOUDF(1) || IsUDF(1) ||
            IsAudio(1));
  }

  void SetFirstTrack(int nTrack) { m_nFirstTrack = nTrack; }
  void SetTrackCount(int nCount) { m_nNumTrack = nCount; }
  void SetFirstAudioTrack(int nTrack) { m_nFirstAudio = nTrack; }
  void SetFirstDataTrack(int nTrack) { m_nFirstData = nTrack; }
  void SetDataTrackCount(int nCount) { m_nNumData = nCount; }
  void SetAudioTrackCount(int nCount) { m_nNumAudio = nCount; }
  void SetTrackInformation(int nTrack, trackinfo nInfo)
  {
    if (nTrack > 0 && nTrack <= 99)
      m_ti[nTrack - 1] = nInfo;
  }
  void SetDiscCDTextInformation(xbmc_cdtext_t cdtext) { m_cdtext = cdtext; }

  void SetCddbDiscId(uint32_t ulCddbDiscId) { m_ulCddbDiscId = ulCddbDiscId; }
  void SetDiscLength(int nLength) { m_nLength = nLength; }
  bool HasCDDBInfo() { return m_bHasCDDBInfo; }
  void SetNoCDDBInfo() { m_bHasCDDBInfo = false; }

  void SetDiscLabel(const std::string& strDiscLabel) { m_strDiscLabel = strDiscLabel; }

private:
  int m_nFirstData; /* # of first data track */
  int m_nNumData; /* # of data tracks */
  int m_nFirstAudio; /* # of first audio track */
  int m_nNumAudio; /* # of audio tracks */
  int m_nNumTrack;
  int m_nFirstTrack;
  trackinfo m_ti[100];
  uint32_t m_ulCddbDiscId;
  int m_nLength; // Disclength can be used for cddb query, also see trackinfo.nFrames
  bool m_bHasCDDBInfo;
  std::string m_strDiscLabel;
  xbmc_cdtext_t m_cdtext; //  CD-Text for this disc
};

class CLibcdio
{
public:
  CLibcdio();
  ~CLibcdio();


  // libcdio is not thread safe so these are wrappers to libcdio routines
  bool cdio_open(const char* psz_source, CdDriver driver_id);
  bool cdio_open_win32(const char* psz_source);
  void cdio_destroy();
  CdDiscMode cdio_get_discmode();
  int mmc_get_tray_status();
  int cdio_eject_media();
  uint8_t cdio_get_last_track_num();
  int cdio_get_track_lsn(uint8_t i_track);
  int cdio_get_track_last_lsn(uint8_t i_track);
  CdDriverReturnCode cdio_read_audio_sectors(void* p_buf, int i_lsn, uint32_t i_blocks);

  std::string GetDeviceFileName();
  int ReadSector(HANDLE hDevice, DWORD dwSector, char* lpczBuffer);
  int ReadSectorMode2(HANDLE hDevice, DWORD dwSector, char* lpczBuffer);
  int ReadSectorCDDA(HANDLE hDevice, DWORD dwSector, char* lpczBuffer);
  void CloseCDROM(HANDLE hDevice);

  void PrintAnalysis(int fs, int num_audio);

  CCdInfo* GetCdInfo(char* cDeviceFileName = NULL);
  void GetCdTextInfo(xbmc_cdtext_t& xcdt, int trackNum);
  int ReadBlock(int superblock, uint32_t offset, uint8_t bufnum, uint8_t track_num);
  bool IsIt(int num);
  int IsHFS(void);
  int Is3DO(void);
  int IsJoliet(void);
  int IsUDF(void);
  int GetSize(void);
  int GetJolietLevel(void);
  int GuessFilesystem(int start_session, uint8_t track_num);
  int GetLibraryVersion() const;

  uint32_t CddbDiscId();
  int CddbDecDigitSum(int n);

private:
  char buffer[7][2352]; /* for CD-Data */
  static signature_t sigs[17];
  int i = 0, j = 0; /* index */
  int m_nStartTrack; /* first sector of track */
  int m_nIsofsSize; /* size of session */
  int m_nJolietLevel;
  int m_nMsOffset; /* multisession offset found by track-walking */
  int m_nDataStart; /* start of data area */
  int m_nFs;
  int m_nUDFVerMinor;
  int m_nUDFVerMajor;

  uint8_t m_nNumTracks = 255;
  uint8_t m_nFirstTrackNum = 255;

  std::string m_strDiscLabel;

  int m_nFirstData; /* # of first data track */
  int m_nNumData; /* # of data tracks */
  int m_nFirstAudio; /* # of first audio track */
  int m_nNumAudio; /* # of audio tracks */

private:
  CdIo_t* m_handle;
  std::string m_defaultDevice;
  static CCriticalSection s_critSection;
};

} // namespace MEDIA_DETECT
