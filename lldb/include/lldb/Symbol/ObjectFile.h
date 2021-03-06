//===-- ObjectFile.h --------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_ObjectFile_h_
#define liblldb_ObjectFile_h_

#include "lldb/lldb-private.h"
#include "lldb/Core/DataExtractor.h"
#include "lldb/Host/FileSpec.h"
#include "lldb/Core/FileSpecList.h"
#include "lldb/Core/ModuleChild.h"
#include "lldb/Core/PluginInterface.h"
#include "lldb/Host/Endian.h"
#include "lldb/Symbol/Symtab.h"
#include "lldb/Symbol/UnwindTable.h"

namespace lldb_private {
    
class ObjectFileJITDelegate
{
public:
    ObjectFileJITDelegate ()
    {
    }
    
    virtual
    ~ObjectFileJITDelegate()
    {
    }
    
    virtual lldb::ByteOrder
    GetByteOrder () const = 0;
    
    virtual uint32_t
    GetAddressByteSize () const = 0;
    
    virtual void
    PopulateSymtab (lldb_private::ObjectFile *obj_file,
                    lldb_private::Symtab &symtab) = 0;
    
    virtual void
    PopulateSectionList (lldb_private::ObjectFile *obj_file,
                         lldb_private::SectionList &section_list) = 0;
    
    virtual bool
    GetArchitecture (lldb_private::ArchSpec &arch) = 0;
};

//----------------------------------------------------------------------
/// @class ObjectFile ObjectFile.h "lldb/Symbol/ObjectFile.h"
/// @brief A plug-in interface definition class for object file parsers.
///
/// Object files belong to Module objects and know how to extract
/// information from executable, shared library, and object (.o) files
/// used by operating system runtime. The symbol table and section list
/// for an object file.
///
/// Object files can be represented by the entire file, or by part of a
/// file. Examples of object files that are part of a file include
/// object files that contain information for multiple architectures in
/// the same file, or archive files that contain multiple objects
/// (ranlib archives) (possibly for multiple architectures as well).
///
/// Object archive files (e.g. ranlib archives) can contain 
/// multiple .o (object) files that must be selected by index or by name. 
/// The number of objects that an ObjectFile contains can be determined 
/// using the ObjectFile::GetNumObjects() const
/// function, and followed by a call to
/// ObjectFile::SelectObjectAtIndex (uint32_t) to change the currently
/// selected object. Objects can also be selected by name using the
/// ObjectFile::SelectObject(const char *) function.
///
/// Once an architecture is selected (and an object is selected for
/// for archives), the object file information can be extracted from
/// this abstract class.
//----------------------------------------------------------------------
class ObjectFile:
    public std::enable_shared_from_this<ObjectFile>,
    public PluginInterface,
    public ModuleChild
{
friend class lldb_private::Module;

public:
    typedef enum 
    {
        eTypeInvalid = 0,
        eTypeCoreFile,      /// A core file that has a checkpoint of a program's execution state
        eTypeExecutable,    /// A normal executable
        eTypeDebugInfo,     /// An object file that contains only debug information
        eTypeDynamicLinker, /// The platform's dynamic linker executable
        eTypeObjectFile,    /// An intermediate object file
        eTypeSharedLibrary, /// A shared library that can be used during execution
        eTypeStubLibrary,   /// A library that can be linked against but not used for execution
        eTypeJIT,           /// JIT code that has symbols, sections and possibly debug info
        eTypeUnknown
    } Type;

    typedef enum 
    {
        eStrataInvalid = 0,
        eStrataUnknown,
        eStrataUser,
        eStrataKernel,
        eStrataRawImage,
        eStrataJIT
    } Strata;

    //------------------------------------------------------------------
    /// Construct with a parent module, offset, and header data.
    ///
    /// Object files belong to modules and a valid module must be
    /// supplied upon construction. The at an offset within a file for
    /// objects that contain more than one architecture or object.
    //------------------------------------------------------------------
    ObjectFile (const lldb::ModuleSP &module_sp, 
                const FileSpec *file_spec_ptr, 
                lldb::offset_t file_offset,
                lldb::offset_t length,
                const lldb::DataBufferSP& data_sp,
                lldb::offset_t data_offset);

    ObjectFile (const lldb::ModuleSP &module_sp, 
                const lldb::ProcessSP &process_sp,
                lldb::addr_t header_addr, 
                lldb::DataBufferSP& data_sp);

    //------------------------------------------------------------------
    /// Destructor.
    ///
    /// The destructor is virtual since this class is designed to be
    /// inherited from by the plug-in instance.
    //------------------------------------------------------------------
    virtual
    ~ObjectFile();
    
    //------------------------------------------------------------------
    /// Dump a description of this object to a Stream.
    ///
    /// Dump a description of the current contents of this object
    /// to the supplied stream \a s. The dumping should include the
    /// section list if it has been parsed, and the symbol table
    /// if it has been parsed.
    ///
    /// @param[in] s
    ///     The stream to which to dump the object description.
    //------------------------------------------------------------------
    virtual void
    Dump (Stream *s) = 0;

    //------------------------------------------------------------------
    /// Find a ObjectFile plug-in that can parse \a file_spec.
    ///
    /// Scans all loaded plug-in interfaces that implement versions of
    /// the ObjectFile plug-in interface and returns the first
    /// instance that can parse the file.
    ///
    /// @param[in] module
    ///     The parent module that owns this object file.
    ///
    /// @param[in] file_spec
    ///     A file specification that indicates which file to use as the
    ///     object file.
    ///
    /// @param[in] file_offset
    ///     The offset into the file at which to start parsing the
    ///     object. This is for files that contain multiple
    ///     architectures or objects.
    ///
    /// @param[in] file_size
    ///     The size of the current object file if it can be determined
    ///     or if it is known. This can be zero.
    ///
    /// @see ObjectFile::ParseHeader()
    //------------------------------------------------------------------
    static lldb::ObjectFileSP
    FindPlugin (const lldb::ModuleSP &module_sp,
                const FileSpec* file_spec,
                lldb::offset_t file_offset,
                lldb::offset_t file_size,
                lldb::DataBufferSP &data_sp,
                lldb::offset_t &data_offset);

    //------------------------------------------------------------------
    /// Find a ObjectFile plug-in that can parse a file in memory.
    ///
    /// Scans all loaded plug-in interfaces that implement versions of
    /// the ObjectFile plug-in interface and returns the first
    /// instance that can parse the file.
    ///
    /// @param[in] module
    ///     The parent module that owns this object file.
    ///
    /// @param[in] process_sp
    ///     A shared pointer to the process whose memory space contains
    ///     an object file. This will be stored as a std::weak_ptr.
    ///
    /// @param[in] header_addr
    ///     The address of the header for the object file in memory.
    //------------------------------------------------------------------
    static lldb::ObjectFileSP
    FindPlugin (const lldb::ModuleSP &module_sp, 
                const lldb::ProcessSP &process_sp,
                lldb::addr_t header_addr,
                lldb::DataBufferSP &file_data_sp);

    
    static size_t
    GetModuleSpecifications (const FileSpec &file,
                             lldb::offset_t file_offset,
                             lldb::offset_t file_size,
                             ModuleSpecList &specs);
    
    static size_t
    GetModuleSpecifications (const lldb_private::FileSpec& file,
                             lldb::DataBufferSP& data_sp,
                             lldb::offset_t data_offset,
                             lldb::offset_t file_offset,
                             lldb::offset_t file_size,
                             lldb_private::ModuleSpecList &specs);
    //------------------------------------------------------------------
    /// Split a path into a file path with object name.
    ///
    /// For paths like "/tmp/foo.a(bar.o)" we often need to split a path
    /// up into the actual path name and into the object name so we can
    /// make a valid object file from it.
    ///
    /// @param[in] path_with_object
    ///     A path that might contain an archive path with a .o file
    ///     specified in parens in the basename of the path.
    ///
    /// @param[out] archive_file
    ///     If \b true is returned, \a file_spec will be filled in with
    ///     the path to the archive.
    ///
    /// @param[out] archive_object
    ///     If \b true is returned, \a object will be filled in with
    ///     the name of the object inside the archive.
    ///
    /// @return
    ///     \b true if the path matches the pattern of archive + object
    ///     and \a archive_file and \a archive_object are modified,
    ///     \b false otherwise and \a archive_file and \a archive_object
    ///     are guaranteed to be remain unchanged.
    //------------------------------------------------------------------
    static bool
    SplitArchivePathWithObject (const char *path_with_object,
                                lldb_private::FileSpec &archive_file,
                                lldb_private::ConstString &archive_object,
                                bool must_exist);

    //------------------------------------------------------------------
    /// Gets the address size in bytes for the current object file.
    ///
    /// @return
    ///     The size of an address in bytes for the currently selected
    ///     architecture (and object for archives). Returns zero if no
    ///     architecture or object has been selected.
    //------------------------------------------------------------------
    virtual uint32_t
    GetAddressByteSize ()  const = 0;

    //------------------------------------------------------------------
    /// Get the address type given a file address in an object file.
    ///
    /// Many binary file formats know what kinds
    /// This is primarily for ARM binaries, though it can be applied to
    /// any executable file format that supports different opcode types
    /// within the same binary. ARM binaries support having both ARM and
    /// Thumb within the same executable container. We need to be able
    /// to get
    /// @return
    ///     The size of an address in bytes for the currently selected
    ///     architecture (and object for archives). Returns zero if no
    ///     architecture or object has been selected.
    //------------------------------------------------------------------
    virtual lldb::AddressClass
    GetAddressClass (lldb::addr_t file_addr);

    //------------------------------------------------------------------
    /// Extract the dependent modules from an object file.
    ///
    /// If an object file has information about which other images it
    /// depends on (such as shared libraries), this function will
    /// provide the list. Since many executables or shared libraries
    /// may depend on the same files,
    /// FileSpecList::AppendIfUnique(const FileSpec &) should be
    /// used to make sure any files that are added are not already in
    /// the list.
    ///
    /// @param[out] file_list
    ///     A list of file specification objects that gets dependent
    ///     files appended to.
    ///
    /// @return
    ///     The number of new files that were appended to \a file_list.
    ///
    /// @see FileSpecList::AppendIfUnique(const FileSpec &)
    //------------------------------------------------------------------
    virtual uint32_t
    GetDependentModules (FileSpecList& file_list) = 0;
    
    //------------------------------------------------------------------
    /// Tells whether this object file is capable of being the main executable
    /// for a process.
    ///
    /// @return
    ///     \b true if it is, \b false otherwise.
    //------------------------------------------------------------------
    virtual bool
    IsExecutable () const = 0;

    //------------------------------------------------------------------
    /// Returns the offset into a file at which this object resides.
    ///
    /// Some files contain many object files, and this function allows
    /// access to an object's offset within the file.
    ///
    /// @return
    ///     The offset in bytes into the file. Defaults to zero for
    ///     simple object files that a represented by an entire file.
    //------------------------------------------------------------------
    virtual lldb::addr_t
    GetFileOffset () const
    { return m_file_offset; }

    virtual lldb::addr_t
    GetByteSize () const
    { return m_length; }

    //------------------------------------------------------------------
    /// Get accessor to the object file specification.
    ///
    /// @return
    ///     The file specification object pointer if there is one, or
    ///     NULL if this object is only from memory.
    //------------------------------------------------------------------
    virtual FileSpec&
    GetFileSpec() { return m_file; }

    //------------------------------------------------------------------
    /// Get const accessor to the object file specification.
    ///
    /// @return
    ///     The const file specification object pointer if there is one,
    ///     or NULL if this object is only from memory.
    //------------------------------------------------------------------
    virtual const FileSpec&
    GetFileSpec() const { return m_file; }

    //------------------------------------------------------------------
    /// Get the name of the cpu, vendor and OS for this object file.
    ///
    /// This value is a string that represents the target triple where
    /// the cpu type, the vendor and the OS are encoded into a string.
    ///
    /// @param[out] target_triple
    ///     The string value of the target triple.
    ///
    /// @return
    ///     \b True if the target triple was able to be computed, \b
    ///     false otherwise.
    //------------------------------------------------------------------
    virtual bool
    GetArchitecture (ArchSpec &arch) = 0;

    //------------------------------------------------------------------
    /// Gets the section list for the currently selected architecture
    /// (and object for archives).
    ///
    /// Section list parsing can be deferred by ObjectFile instances
    /// until this accessor is called the first time.
    ///
    /// @return
    ///     The list of sections contained in this object file.
    //------------------------------------------------------------------
    virtual SectionList *
    GetSectionList ();

    virtual void
    CreateSections (SectionList &unified_section_list) = 0;

    //------------------------------------------------------------------
    /// Gets the symbol table for the currently selected architecture
    /// (and object for archives).
    ///
    /// Symbol table parsing can be deferred by ObjectFile instances
    /// until this accessor is called the first time.
    ///
    /// @return
    ///     The symbol table for this object file.
    //------------------------------------------------------------------
    virtual Symtab *
    GetSymtab () = 0;

    //------------------------------------------------------------------
    /// Appends a Symbol for the specified so_addr to the symbol table.
    ///
    /// If verify_unique is false, the symbol table is not searched
    /// to determine if a Symbol found at this address has already been
    /// added to the symbol table.  When verify_unique is true, this
    /// method resolves the Symbol as the first match in the SymbolTable
    /// and appends a Symbol only if required/found.
    ///
    /// @return
    ///     The resolved symbol or nullptr.  Returns nullptr if a
    ///     a Symbol could not be found for the specified so_addr.
    //------------------------------------------------------------------
    virtual Symbol *
    ResolveSymbolForAddress(const Address &so_addr, bool verify_unique)
    {
        // Typically overridden to lazily add stripped symbols recoverable from
        // the exception handling unwind information (i.e. without parsing
        // the entire eh_frame section.
        //
        // The availability of LC_FUNCTION_STARTS allows ObjectFileMachO
        // to efficiently add stripped symbols when the symbol table is
        // first constructed.  Poorer cousins are PECoff and ELF.
        return nullptr;
    }

    //------------------------------------------------------------------
    /// Detect if this object file has been stripped of local symbols.
    //------------------------------------------------------------------
    /// Detect if this object file has been stripped of local symbols.
    ///
    /// @return
    ///     Return \b true if the object file has been stripped of local
    ///     symbols.
    //------------------------------------------------------------------
    virtual bool
    IsStripped () = 0;

    //------------------------------------------------------------------
    /// Frees the symbol table.
    ///
    /// This function should only be used when an object file is
    ///
    /// @param[in] flags
    ///     eSymtabFromUnifiedSectionList: Whether to clear symbol table
    ///     for unified module section list, or object file.
    ///
    /// @return
    ///     The symbol table for this object file.
    //------------------------------------------------------------------
    virtual void
    ClearSymtab ();
    
    //------------------------------------------------------------------
    /// Gets the UUID for this object file.
    ///
    /// If the object file format contains a UUID, the value should be
    /// returned. Else ObjectFile instances should return the MD5
    /// checksum of all of the bytes for the object file (or memory for
    /// memory based object files).
    ///
    /// @return
    ///     Returns \b true if a UUID was successfully extracted into
    ///     \a uuid, \b false otherwise.
    //------------------------------------------------------------------
    virtual bool
    GetUUID (lldb_private::UUID* uuid) = 0;

    //------------------------------------------------------------------
    /// Gets the symbol file spec list for this object file.
    ///
    /// If the object file format contains a debug symbol file link,
    /// the values will be returned in the FileSpecList.
    ///
    /// @return
    ///     Returns filespeclist.
    //------------------------------------------------------------------
    virtual lldb_private::FileSpecList
    GetDebugSymbolFilePaths()
    {
        return FileSpecList();
    }

    //------------------------------------------------------------------
    /// Gets the file spec list of libraries re-exported by this object file.
    ///
    /// If the object file format has the notion of one library re-exporting the symbols from another,
    /// the re-exported libraries will be returned in the FileSpecList.
    ///
    /// @return
    ///     Returns filespeclist.
    //------------------------------------------------------------------
    virtual lldb_private::FileSpecList
    GetReExportedLibraries ()
    {
        return FileSpecList();
    }
    
    //------------------------------------------------------------------
    /// Sets the load address for an entire module, assuming a rigid
    /// slide of sections, if possible in the implementation.
    ///
    /// @return
    ///     Returns true iff any section's load address changed.
    //------------------------------------------------------------------
    virtual bool
    SetLoadAddress(Target &target,
                   lldb::addr_t value,
                   bool value_is_offset)
    {
        return false;
    }

    //------------------------------------------------------------------
    /// Gets whether endian swapping should occur when extracting data
    /// from this object file.
    ///
    /// @return
    ///     Returns \b true if endian swapping is needed, \b false
    ///     otherwise.
    //------------------------------------------------------------------
    virtual lldb::ByteOrder
    GetByteOrder () const = 0;

    //------------------------------------------------------------------
    /// Attempts to parse the object header.
    ///
    /// This function is used as a test to see if a given plug-in
    /// instance can parse the header data already contained in
    /// ObjectFile::m_data. If an object file parser does not
    /// recognize that magic bytes in a header, false should be returned
    /// and the next plug-in can attempt to parse an object file.
    ///
    /// @return
    ///     Returns \b true if the header was parsed successfully, \b
    ///     false otherwise.
    //------------------------------------------------------------------
    virtual bool
    ParseHeader () = 0;

    //------------------------------------------------------------------
    /// Returns a reference to the UnwindTable for this ObjectFile
    ///
    /// The UnwindTable contains FuncUnwinders objects for any function in
    /// this ObjectFile.  If a FuncUnwinders object hasn't been created yet
    /// (i.e. the function has yet to be unwound in a stack walk), it
    /// will be created when requested.  Specifically, we do not create
    /// FuncUnwinders objects for functions until they are needed.
    ///
    /// @return
    ///     Returns the unwind table for this object file.
    //------------------------------------------------------------------
    virtual lldb_private::UnwindTable&
    GetUnwindTable () { return m_unwind_table; }

    //------------------------------------------------------------------
    /// Similar to Process::GetImageInfoAddress().
    ///
    /// Some platforms embed auxiliary structures useful to debuggers in the
    /// address space of the inferior process.  This method returns the address
    /// of such a structure if the information can be resolved via entries in
    /// the object file.  ELF, for example, provides a means to hook into the
    /// runtime linker so that a debugger may monitor the loading and unloading
    /// of shared libraries.
    ///
    /// @return 
    ///     The address of any auxiliary tables, or an invalid address if this
    ///     object file format does not support or contain such information.
    virtual lldb_private::Address
    GetImageInfoAddress (Target *target) { return Address(); }
    
    //------------------------------------------------------------------
    /// Returns the address of the Entry Point in this object file - if
    /// the object file doesn't have an entry point (because it is not an
    /// executable file) then an invalid address is returned.
    ///
    /// @return
    ///     Returns the entry address for this module.
    //------------------------------------------------------------------
    virtual lldb_private::Address
    GetEntryPointAddress () { return Address();}

    //------------------------------------------------------------------
    /// Returns the address that represents the header of this object
    /// file.
    ///
    /// The header address is defined as where the header for the object
    /// file is that describes the content of the file. If the header
    /// doesn't appear in a section that is defined in the object file,
    /// an address with no section is returned that has the file offset
    /// set in the m_file_offset member of the lldb_private::Address object.
    ///
    /// @return
    ///     Returns the entry address for this module.
    //------------------------------------------------------------------
    virtual lldb_private::Address
    GetHeaderAddress () { return Address(m_memory_addr);}

    
    virtual uint32_t
    GetNumThreadContexts ()
    {
        return 0;
    }

    virtual lldb::RegisterContextSP
    GetThreadContextAtIndex (uint32_t idx, lldb_private::Thread &thread)
    {
        return lldb::RegisterContextSP();
    }

    //------------------------------------------------------------------
    /// The object file should be able to calculate its type by looking
    /// at its file header and possibly the sections or other data in
    /// the object file. The file type is used in the debugger to help
    /// select the correct plug-ins for the job at hand, so this is 
    /// important to get right. If any eTypeXXX definitions do not match
    /// up with the type of file you are loading, please feel free to
    /// add a new enumeration value.
    ///
    /// @return
    ///     The calculated file type for the current object file.
    //------------------------------------------------------------------
    virtual Type
    CalculateType() = 0;

    //------------------------------------------------------------------
    /// In cases where the type can't be calculated (elf files), this
    /// routine allows someone to explicitly set it. As an example,
    /// SymbolVendorELF uses this routine to set eTypeDebugInfo when
    /// loading debug link files.
    virtual void
    SetType (Type type)
    {
        m_type = type;
    }

    //------------------------------------------------------------------
    /// The object file should be able to calculate the strata of the
    /// object file.
    ///
    /// Many object files for platforms might be for either user space
    /// debugging or for kernel debugging. If your object file subclass
    /// can figure this out, it will help with debugger plug-in selection
    /// when it comes time to debug.
    ///
    /// @return
    ///     The calculated object file strata for the current object 
    ///     file.
    //------------------------------------------------------------------
    virtual Strata
    CalculateStrata() = 0;
    
    //------------------------------------------------------------------
    /// Get the object file version numbers.
    ///
    /// Many object files have a set of version numbers that describe
    /// the version of the executable or shared library. Typically there
    /// are major, minor and build, but there may be more. This function
    /// will extract the versions from object files if they are available.
    ///
    /// If \a versions is NULL, or if \a num_versions is 0, the return
    /// value will indicate how many version numbers are available in
    /// this object file. Then a subsequent call can be made to this 
    /// function with a value of \a versions and \a num_versions that
    /// has enough storage to store some or all version numbers.
    ///
    /// @param[out] versions
    ///     A pointer to an array of uint32_t types that is \a num_versions
    ///     long. If this value is NULL, the return value will indicate
    ///     how many version numbers are required for a subsequent call
    ///     to this function so that all versions can be retrieved. If
    ///     the value is non-NULL, then at most \a num_versions of the
    ///     existing versions numbers will be filled into \a versions.
    ///     If there is no version information available, \a versions
    ///     will be filled with \a num_versions UINT32_MAX values
    ///     and zero will be returned.
    ///
    /// @param[in] num_versions
    ///     The maximum number of entries to fill into \a versions. If
    ///     this value is zero, then the return value will indicate
    ///     how many version numbers there are in total so another call
    ///     to this function can be make with adequate storage in
    ///     \a versions to get all of the version numbers. If \a
    ///     num_versions is less than the actual number of version 
    ///     numbers in this object file, only \a num_versions will be
    ///     filled into \a versions (if \a versions is non-NULL).
    ///
    /// @return
    ///     This function always returns the number of version numbers
    ///     that this object file has regardless of the number of
    ///     version numbers that were copied into \a versions. 
    //------------------------------------------------------------------
    virtual uint32_t
    GetVersion (uint32_t *versions, uint32_t num_versions)
    {
        if (versions && num_versions)
        {
            for (uint32_t i=0; i<num_versions; ++i)
                versions[i] = UINT32_MAX;
        }
        return 0;
    }
    
    //------------------------------------------------------------------
    /// Get the minimum OS version this object file can run on.
    ///
    /// Some object files have information that specifies the minimum OS
    /// version that they can be used on.
    ///
    /// If \a versions is NULL, or if \a num_versions is 0, the return
    /// value will indicate how many version numbers are available in
    /// this object file. Then a subsequent call can be made to this
    /// function with a value of \a versions and \a num_versions that
    /// has enough storage to store some or all version numbers.
    ///
    /// @param[out] versions
    ///     A pointer to an array of uint32_t types that is \a num_versions
    ///     long. If this value is NULL, the return value will indicate
    ///     how many version numbers are required for a subsequent call
    ///     to this function so that all versions can be retrieved. If
    ///     the value is non-NULL, then at most \a num_versions of the
    ///     existing versions numbers will be filled into \a versions.
    ///     If there is no version information available, \a versions
    ///     will be filled with \a num_versions UINT32_MAX values
    ///     and zero will be returned.
    ///
    /// @param[in] num_versions
    ///     The maximum number of entries to fill into \a versions. If
    ///     this value is zero, then the return value will indicate
    ///     how many version numbers there are in total so another call
    ///     to this function can be make with adequate storage in
    ///     \a versions to get all of the version numbers. If \a
    ///     num_versions is less than the actual number of version
    ///     numbers in this object file, only \a num_versions will be
    ///     filled into \a versions (if \a versions is non-NULL).
    ///
    /// @return
    ///     This function always returns the number of version numbers
    ///     that this object file has regardless of the number of
    ///     version numbers that were copied into \a versions.
    //------------------------------------------------------------------
    virtual uint32_t
    GetMinimumOSVersion (uint32_t *versions, uint32_t num_versions)
    {
        if (versions && num_versions)
        {
            for (uint32_t i=0; i<num_versions; ++i)
                versions[i] = UINT32_MAX;
        }
        return 0;
    }

    //------------------------------------------------------------------
    /// Get the SDK OS version this object file was built with.
    ///
    /// The versions arguments and returns values are the same as the
    /// GetMinimumOSVersion()
    //------------------------------------------------------------------
    virtual uint32_t
    GetSDKVersion (uint32_t *versions, uint32_t num_versions)
    {
        if (versions && num_versions)
        {
            for (uint32_t i=0; i<num_versions; ++i)
                versions[i] = UINT32_MAX;
        }
        return 0;
    }

    //------------------------------------------------------------------
    // Member Functions
    //------------------------------------------------------------------
    Type
    GetType ()
    {
        if (m_type == eTypeInvalid)
            m_type = CalculateType();
        return m_type;
    }
    
    Strata
    GetStrata ()
    {
        if (m_strata == eStrataInvalid)
            m_strata = CalculateStrata();
        return m_strata;
    }
    
    // When an object file is in memory, subclasses should try and lock
    // the process weak pointer. If the process weak pointer produces a
    // valid ProcessSP, then subclasses can call this function to read
    // memory.
    static lldb::DataBufferSP
    ReadMemory (const lldb::ProcessSP &process_sp, 
                lldb::addr_t addr, 
                size_t byte_size);

    size_t
    GetData (lldb::offset_t offset, size_t length, DataExtractor &data) const;
    
    size_t
    CopyData (lldb::offset_t offset, size_t length, void *dst) const;
    
    virtual size_t
    ReadSectionData (const Section *section, 
                     lldb::offset_t section_offset, 
                     void *dst, 
                     size_t dst_len) const;
    virtual size_t
    ReadSectionData (const Section *section, 
                     DataExtractor& section_data) const;
    
    size_t
    MemoryMapSectionData (const Section *section, 
                          DataExtractor& section_data) const;
    
    bool
    IsInMemory () const
    {
        return m_memory_addr != LLDB_INVALID_ADDRESS;
    }
    
protected:
    //------------------------------------------------------------------
    // Member variables.
    //------------------------------------------------------------------
    FileSpec m_file;
    Type m_type;
    Strata m_strata;
    lldb::addr_t m_file_offset; ///< The offset in bytes into the file, or the address in memory
    lldb::addr_t m_length; ///< The length of this object file if it is known (can be zero if length is unknown or can't be determined).
    DataExtractor m_data; ///< The data for this object file so things can be parsed lazily.
    lldb_private::UnwindTable m_unwind_table; /// < Table of FuncUnwinders objects created for this ObjectFile's functions
    lldb::ProcessWP m_process_wp;
    const lldb::addr_t m_memory_addr;
    std::unique_ptr<lldb_private::SectionList> m_sections_ap;
    std::unique_ptr<lldb_private::Symtab> m_symtab_ap;
    
    //------------------------------------------------------------------
    /// Sets the architecture for a module.  At present the architecture
    /// can only be set if it is invalid.  It is not allowed to switch from
    /// one concrete architecture to another.
    /// 
    /// @param[in] new_arch
    ///     The architecture this module will be set to.
    ///
    /// @return
    ///     Returns \b true if the architecture was changed, \b
    ///     false otherwise.
    //------------------------------------------------------------------
    bool SetModulesArchitecture (const ArchSpec &new_arch);

private:
    DISALLOW_COPY_AND_ASSIGN (ObjectFile);
};

} // namespace lldb_private

#endif  // liblldb_ObjectFile_h_

