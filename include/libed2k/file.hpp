
#ifndef __FILE__HPP__
#define __FILE__HPP__

#include <string>
#include <vector>
#include <deque>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>

#include "libed2k/config.hpp"
#include "libed2k/error_code.hpp"
#include "libed2k/size_type.hpp"
#include "libed2k/alert.hpp"
#include "libed2k/filesystem.hpp"
#include "libed2k/md4_hash.hpp"
#include "libed2k/packet_struct.hpp"
#include "libed2k/alert_types.hpp"

namespace libed2k
{

    ///////////////////////////////////////////////////////////////////////////////
    // ED2K File Type
    //
    enum EED2KFileType
    {
        ED2KFT_ANY              = 0,
        ED2KFT_AUDIO            = 1,    // ED2K protocol value (eserver 17.6+)
        ED2KFT_VIDEO            = 2,    // ED2K protocol value (eserver 17.6+)
        ED2KFT_IMAGE            = 3,    // ED2K protocol value (eserver 17.6+)
        ED2KFT_PROGRAM          = 4,    // ED2K protocol value (eserver 17.6+)
        ED2KFT_DOCUMENT         = 5,    // ED2K protocol value (eserver 17.6+)
        ED2KFT_ARCHIVE          = 6,    // ED2K protocol value (eserver 17.6+)
        ED2KFT_CDIMAGE          = 7,    // ED2K protocol value (eserver 17.6+)
        ED2KFT_EMULECOLLECTION  = 8
    };

    // Media values for FT_FILETYPE
    const std::string ED2KFTSTR_AUDIO("Audio");
    const std::string ED2KFTSTR_VIDEO("Video");
    const std::string ED2KFTSTR_IMAGE("Image");
    const std::string ED2KFTSTR_DOCUMENT("Doc");
    const std::string ED2KFTSTR_PROGRAM("Pro");
    const std::string ED2KFTSTR_ARCHIVE("Arc");  // *Mule internal use only
    const std::string ED2KFTSTR_CDIMAGE("Iso");  // *Mule internal use only
    const std::string ED2KFTSTR_EMULECOLLECTION("EmuleCollection");
    const std::string ED2KFTSTR_FOLDER("Folder"); // Value for eD2K tag FT_FILETYPE
    const std::string ED2KFTSTR_USER("User"); // eMule internal use only

    // Additional media meta data tags from eDonkeyHybrid (note also the uppercase/lowercase)
    const std::string FT_ED2K_MEDIA_ARTIST("Artist");    // <string>
    const std::string FT_ED2K_MEDIA_ALBUM(     "Album");     // <string>
    const std::string FT_ED2K_MEDIA_TITLE(     "Title");     // <string>
    const std::string FT_ED2K_MEDIA_LENGTH(    "length");    // <string> !!!
    const std::string FT_ED2K_MEDIA_BITRATE(   "bitrate");   // <uint32>
    const std::string FT_ED2K_MEDIA_CODEC(     "codec");    // <string>
    // ?
    #define TAG_NSENT               "# Sent"
    #define TAG_ONIP                "ip"
    #define TAG_ONPORT              "port"



    const boost::uint8_t PR_VERYLOW     = 4;
    const boost::uint8_t PR_LOW         = 0;
    const boost::uint8_t PR_NORMAL      = 1;
    const boost::uint8_t PR_HIGH        = 2;
    const boost::uint8_t PR_VERYHIGH    = 3;
    const boost::uint8_t PR_AUTO        = 5;
    const boost::uint8_t PR_POWERSHARE  = 6;

    extern EED2KFileType GetED2KFileTypeID(const std::string& strFileName);
    extern std::string GetED2KFileTypeSearchTerm(EED2KFileType iFileID);
    extern EED2KFileType GetED2KFileTypeSearchID(EED2KFileType iFileID);
    extern std::string GetFileTypeByName(const std::string& strFileName);

    const unsigned int PIECE_COUNT_ALLOC = 20;

    // for future usage
    enum PreferencesDatFileVersions {
        PREFFILE_VERSION        = 0x14 //<<-- last change: reduced .dat, by using .ini
    };

    enum PartMetFileVersions {
        PARTFILE_VERSION            = 0xe0,
        PARTFILE_SPLITTEDVERSION    = 0xe1, // For edonkey part files importing.
        PARTFILE_VERSION_LARGEFILE  = 0xe2
    };

    enum CreditFileVersions {
        CREDITFILE_VERSION      = 0x12
    };

    enum CanceledFileListVersions {
        CANCELEDFILE_VERSION    = 0x21
    };

    // known.met file header
    const boost::uint8_t  MET_HEADER                  = 0x0E;
    const boost::uint8_t  MET_HEADER_WITH_LARGEFILES  = 0x0F;

    typedef container_holder<boost::uint16_t, std::vector<md4_hash> > hash_list;

    /**
      * simple known file entry structure
     */
    struct known_file_entry
    {
        boost::uint32_t             m_nLastChanged; //!< date last changed
        md4_hash                    m_hFile;        //!< file hash
        hash_list                   m_hash_list;
        tag_list<boost::uint32_t>   m_list;

        known_file_entry();

        known_file_entry(const md4_hash& hFile,
                            const std::vector<md4_hash>& hSet,
                            const std::string& filename,
                            size_type  nFilesize,
                            boost::uint32_t nAccepted,
                            boost::uint32_t nRequested,
                            boost::uint64_t nTransferred,
                            boost::uint8_t  nPriority);

        template<typename Archive>
        void serialize(Archive& ar)
        {
            ar & m_nLastChanged;
            ar & m_hFile;
            ar & m_hash_list;
            ar & m_list;
        }

        void dump() const;
    };

    typedef container_holder<boost::uint32_t, std::deque<known_file_entry> > known_file_list;

    /**
      * full known.met file content
     */
    struct known_file_collection
    {
        boost::uint8_t  m_nHeader;
        known_file_list m_known_file_list;

        known_file_collection();
        bool extract_transfer_params(time_t, add_transfer_params& atp);

        template<typename Archive>
        void save(Archive& ar)
        {
            if (m_nHeader != MET_HEADER && m_nHeader != MET_HEADER_WITH_LARGEFILES)
            {
                // incorrect header
                throw libed2k_exception(errors::known_file_invalid_header);
            }

            ar & m_nHeader;
            ar & m_known_file_list;
        }

        template<typename Archive>
        void load(Archive& ar)
        {
            ar & m_nHeader;

            if (m_nHeader != MET_HEADER && m_nHeader != MET_HEADER_WITH_LARGEFILES)
            {
                // incorrect header
                throw libed2k_exception(errors::known_file_invalid_header);
            }

            ar & m_known_file_list;
        }


        void dump() const;

        LIBED2K_SERIALIZATION_SPLIT_MEMBER()
    };

    /**
      * structure for save transfer resume data and additional info like hash, filepath, filesize
     */
    struct transfer_resume_data
    {
        md4_hash    m_hash;     //!< transfer hash
        container_holder<boost::uint16_t, std::string> m_filename; //!< utf-8 file path
        size_type     m_filesize; //!< boost::uint64_t file size
        tag_list<boost::uint8_t>    m_fast_resume_data;
        transfer_resume_data();
        transfer_resume_data(const md4_hash& hash, const std::string& filename, size_type size, const std::vector<char>& fr_data);

        template<typename Archive>
        void serialize(Archive& ar)
        {
            ar & m_hash;
            ar & m_filename;
            ar & m_filesize;
            ar & m_fast_resume_data;
        }
    };

    /**
      * status struct contains error code and operation progress
     */
    struct hash_status
    {
        error_code          m_error;
        std::pair<int,int>  m_progress; //!< current piece, total pieces
        hash_status() : m_error(errors::no_error){}
        hash_status(error_code ec, std::pair<int, int> p) : m_error(ec), m_progress(p) {}
        bool valid() const { return !m_error; }
        bool completed() const { return ((m_progress.first != 0) && (m_progress.first == m_progress.second)); }
        bool operator==(const hash_status& hs) const
        {
            return (m_error == hs.m_error) && (m_progress == hs.m_progress);
        }
    };

    /**
      * simple monitor object
     */
    template <typename Data>
    class monitor_order : boost::noncopyable
    {
    public:
        monitor_order()
        {
            m_cancelled = false;
        }

        void push(const Data& data)
        {
            boost::mutex::scoped_lock lock(m_monitorMutex);
            // for extremely effective we don't check item already exists in deque
            m_queue.push_front(data);
            m_signal.notify_one();
        }

        /**
          * remove all items and abort waiting
         */
        void abort()
        {
            DBG("monitor_order {cancel}");
            boost::mutex::scoped_lock lock(m_monitorMutex);
            std::deque<Data> empty;
            std::swap(m_queue, empty );
            m_cancelled = true;
            m_signal.notify_one();
        }

        /**
          * cancel one item
         */
        void cancel(const Data& data)
        {
            boost::mutex::scoped_lock lock(m_monitorMutex);
            m_queue.erase(std::remove(m_queue.begin(), m_queue.end(), data), m_queue.end());
        }

        size_t size()
        {
            boost::mutex::scoped_lock lock(m_monitorMutex);
            return m_queue.size();
        }

        void reset()
        {
            boost::mutex::scoped_lock lock(m_monitorMutex);
            m_cancelled = false;
        }

        /**
          * returns true when queue will produce data
         */
        bool pop_wait(Data& data)
        {
            boost::mutex::scoped_lock lock(m_monitorMutex);

            if (m_cancelled) { return false; }

            if(m_queue.empty())
            {
                m_signal.wait(lock);
            }

            if (m_queue.empty()) { return false; }

            data = m_queue.back();
            m_queue.pop_back();
            return true;
        }

    private:
        bool                m_cancelled;
        std::deque<Data>    m_queue;
        boost::mutex        m_monitorMutex;
        boost::condition    m_signal;
    };

    class transfer_params_maker
    {
    public:
        transfer_params_maker(alert_manager& am, const std::string& known_filepath);
        virtual ~transfer_params_maker();
        bool start();
        void stop();
        void operator()();

        size_t queue_size() { return m_order.size(); }
        /**
          * @param filepath in UTF-8
         */
        void make_transfer_params(const std::string& filepath);
        void cancel_transfer_params(const std::string& filepath);
    protected:
        virtual void process_item(const std::string& filepath);
        alert_manager& m_am;
    private:
        std::string m_known_filepath;
        known_file_collection m_kfc;
        boost::shared_ptr<boost::thread> m_thread;
        boost::mutex m_mutex;
        monitor_order<std::string> m_order;

    };

    /**
      * structure for save/load binary emulecollection files
     */
    struct emule_binary_collection
    {
        boost::uint32_t m_nVersion;
        tag_list<boost::uint32_t>   m_list;
        container_holder<boost::uint32_t, std::vector<tag_list<boost::uint32_t> > >   m_files;

        template<typename Archive>
        void serialize(Archive& ar)
        {
            ar & m_nVersion;
            ar & m_list;
            ar & m_files;
        }

        bool operator==(const emule_binary_collection& ec) const;
        void dump() const;
    };

    /**
      * one file collection entry
     */
    struct emule_collection_entry
    {
        emule_collection_entry() : m_filename(""), m_filesize(0)
        {
        }

        emule_collection_entry(const std::string& strFilename, size_type nFilesize, const md4_hash& hash) :
            m_filename(strFilename), m_filesize(nFilesize), m_filehash(hash) {}

        bool operator==(const emule_collection_entry& ce) const
        {
            return (m_filename == ce.m_filename &&
                    m_filesize == ce.m_filesize &&
                    m_filehash == ce.m_filehash);
        }

        bool defined() const
        {
            return (!m_filename.empty() && m_filesize != 0 && m_filehash.defined());
        }

        std::string     m_filename;
        size_type       m_filesize;
        md4_hash        m_filehash;
    };

    /**
      * files collection
     */
    struct emule_collection
    {
        /**
          * restore collection from file
         */
        static emule_collection fromFile(const std::string& strFilename);

        /**
          * generate ed2k link from collection item
         */
        static std::string toLink(const std::string& strFilename, size_type nFilesize, const md4_hash& hFile);
        static emule_collection_entry fromLink(const std::string& strLink);

        bool save(const std::string& strFilename, bool binary = false);

        /**
          * add known file
         */
        bool add_file(const std::string& strFilename, size_type nFilesize, const std::string& strFilehash);
        bool add_link(const std::string& strLink);

        const std::string get_ed2k_link(size_t nIndex);
        bool operator==(const emule_collection& ecoll) const;
        std::string                         m_name;
        std::deque<emule_collection_entry>  m_files;
    };
}

#endif //__FILE__HPP__
