/* $Id: UnattendedScript.h 68162 2017-07-28 15:28:33Z vboxsync $ */
/** @file
 * Implementeation of algorithms which read/parse/save scripts for unattended installation.
 */

/*
 * Copyright (C) 2006-2017 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */

#ifndef ____H_UNATTENDEDSCRIPT
#define ____H_UNATTENDEDSCRIPT

#include "VirtualBoxBase.h"
#include <iprt/cpp/utils.h>

using namespace xml;

class Unattended;


/**
 * Base class for all the script readers/editors.
 *
 * @todo get rid of this bugger
 */
class AbstractScript : public RTCNonCopyable
{
protected:
    /** For setting errors.
     * Yeah, class isn't entirely abstract now.  */
    VirtualBoxBase *mpSetError;

private: /* no default constructors for children. */
    AbstractScript() {}

public:
    AbstractScript(VirtualBoxBase *pSetError) : mpSetError(pSetError) {}
    virtual ~AbstractScript() {}

    /**
     * Read a script from a file
     */
    virtual HRESULT read(const Utf8Str &rStrFilename) = 0;

    /**
     * Read a script from a VFS file handle.
     */
    virtual HRESULT readFromHandle(RTVFSFILE hVfsFile, const char *pszFilename) = 0;

    /**
     * Parse the script
     */
    virtual HRESULT parse() = 0;

    /**
     * Save a script to a string.
     *
     * This is used by save() and later others to deloy the script.
     */
    virtual HRESULT saveToString(Utf8Str &rStrDst) = 0;

    /**
     * Save a script to a file.
     * @param   rStrPath        Where to save the script.  This normally points to a
     *                          file, but in a number of child use cases it's
     *                          actually giving a directory to put the script in
     *                          using the default deloyment filename.  One day we
     *                          might make the caller do this path joining.
     * @param   fOverwrite      Whether to overwrite the file or not.
     */
    virtual HRESULT save(const Utf8Str &rStrPath, bool fOverwrite) = 0;

    /**
     * Path where an actual script with user's data is located
     */
    virtual const Utf8Str &getActualScriptPath() const = 0;
};

/**
 * Base class for text based script readers/editors.
 *
 * This deals with reading the file into a string data member, writing it back
 * out to a file, and remember the filenames.
 */
class BaseTextScript : public AbstractScript
{
protected:
    const char * const mpszDefaultTemplateFilename; /**< The default template filename.  Can be empty. */
    const char * const mpszDefaultFilename;         /**< Filename to use when someone calls save() with a directory path.  Can be NULL. */
    RTCString   mStrScriptFullContent;  /**< Raw text file content.  Produced by read() and typically only used by parse(). */
    Utf8Str     mStrOriginalPath;       /**< Path where an original script is located (set by read()). */
    Utf8Str     mStrSavedPath;          /**< Path where an saved script with user's data is located (set by save()). */

public:
    BaseTextScript(VirtualBoxBase *pSetError, const char *pszDefaultTemplateFilename, const char *pszDefaultFilename)
        : AbstractScript(pSetError)
        , mpszDefaultTemplateFilename(pszDefaultTemplateFilename)
        , mpszDefaultFilename(pszDefaultFilename)
    { }
    virtual ~BaseTextScript() {}

    HRESULT read(const Utf8Str &rStrFilename);
    HRESULT readFromHandle(RTVFSFILE hVfsFile, const char *pszFilename);
    HRESULT save(const Utf8Str &rStrFilename, bool fOverwrite);

    /**
     * Gets the default filename for this class of scripts (empty if none).
     *
     * @note Just the filename, no path.
     */
    const char *getDefaultFilename() const
    {
        return mpszDefaultFilename;
    }

    /**
     * Gets the default template filename for this class of scripts (empty if none).
     *
     * @note Just the filename, no path.
     */
    const char *getDefaultTemplateFilename() const
    {
        return mpszDefaultTemplateFilename;
    }

    /**
     * Path to the file we last saved the script as.
     */
    const Utf8Str &getActualScriptPath() const
    {
        return mStrSavedPath;
    }

    /**
     * Path where an original script is located
     */
    const Utf8Str &getOriginalScriptPath() const
    {
        return mStrOriginalPath;
    }
};


/**
 * Generic unattended text script template editor.
 *
 * This just perform variable replacements, no other editing possible.
 *
 * Everything happens during saveToString, parse is a noop.
 */
class UnattendedScriptTemplate : public BaseTextScript
{
protected:
    /** Where to get the replacement strings from. */
    Unattended *mpUnattended;

public:
    UnattendedScriptTemplate(Unattended *pUnattended, const char *pszDefaultTemplateFilename, const char *pszDefaultFilename);
    virtual ~UnattendedScriptTemplate()             {}

    HRESULT parse()                                 { return S_OK; }
    HRESULT saveToString(Utf8Str &rStrDst);

protected:
    /**
     * Gets the replacement value for the given placeholder.
     *
     * @returns COM status code.
     * @param   pachPlaceholder The placholder string.  Not zero terminated.
     * @param   cchPlaceholder  The length of the placeholder.
     * @param   fOutputting     Indicates whether we actually need the correct value
     *                          or is just syntax checking excluded template parts.
     * @param   rValue          Where to return the value.
     */
    HRESULT getReplacement(const char *pachPlaceholder, size_t cchPlaceholder, bool fOutputting, RTCString &rValue);

    /**
     * Overridable worker for getReplacement.
     *
     * @returns COM status code.
     * @param   pachPlaceholder     The placholder string.  Not zero terminated.
     * @param   cchPlaceholder      The length of the placeholder.
     * @param   cchFullPlaceholder  The full placeholder length, including suffixes
     *                              indicating how it should be escaped (for error
     *                              messages).
     * @param   fOutputting         Indicates whether we actually need the correct
     *                              value or is just syntax checking excluded
     *                              template parts.  Intended for voiding triggering
     *                              sanity checks regarding which replacements
     *                              should be used and not (e.g. no guest additions
     *                              path when installing GAs aren't enabled).
     * @param   rValue              Where to return the value.
     * @throws  std::bad_alloc
     */
    virtual HRESULT getUnescapedReplacement(const char *pachPlaceholder, size_t cchPlaceholder,
                                            size_t cchFullPlaceholder, bool fOutputting, RTCString &rValue);


    /**
     * Get the result of a conditional.
     *
     * @returns COM status code.
     * @param   pachPlaceholder     The placholder string.  Not zero terminated.
     * @param   cchPlaceholder      The length of the placeholder.
     * @param   pfOutputting        Where to return the result of the conditional.
     *                              This holds the current outputting state on input
     *                              in case someone want to sanity check anything.
     */
    virtual HRESULT getConditional(const char *pachPlaceholder, size_t cchPlaceholder, bool *pfOutputting);
};


/**
 * Generic line based text script editor.
 *
 * This is used for editing isolinux configuratin files among other things.
 */
class GeneralTextScript : public BaseTextScript
{
protected:
    RTCList<RTCString>  mScriptContentByLines; /**< Content index by line. This contains the edited version. */
    bool                mfDataParsed;          /**< Indicates whether the script has been parse() yet.  */

public:
    GeneralTextScript(VirtualBoxBase *pSetError, const char *pszDefaultTemplateFilename = NULL, const char *pszDefaultFilename = NULL)
        : BaseTextScript(pSetError, pszDefaultTemplateFilename, pszDefaultFilename), mfDataParsed(false)
    {}
    virtual ~GeneralTextScript() {}

    HRESULT parse();
    HRESULT saveToString(Utf8Str &rStrDst);

    //////////////////New functions//////////////////////////////

    bool isDataParsed() const
    {
        return mfDataParsed;
    }

    /**
     * Returns the actual size of script in lines
     */
    size_t getLineNumbersOfScript() const
    {
        return mScriptContentByLines.size();
    }

    /**
     * Gets a read-only reference to the given line, returning Utf8Str::Empty if
     * idxLine is out of range.
     *
     * @returns Line string reference or Utf8Str::Empty.
     * @param   idxLine     The line number.
     *
     * @todo    RTCList doesn't allow this method to be const.
     */
    RTCString const &getContentOfLine(size_t idxLine);

    /**
     * Set new content of line
     */
    HRESULT setContentOfLine(size_t idxLine, const Utf8Str &newContent);

    /**
     * Find a substring in the script
     * Returns a list with the found lines
     * @throws std::bad_alloc
     */
    std::vector<size_t> findTemplate(const Utf8Str &rStrNeedle, RTCString::CaseSensitivity enmCase = RTCString::CaseSensitive);

    /**
     * In line @a idxLine replace the first occurence of @a rStrNeedle with
     * @a rStrRelacement.
     */
    HRESULT findAndReplace(size_t idxLine, const Utf8Str &rStrNeedle, const Utf8Str &rStrReplacement);

    /**
     * Append a string into the end of the given line.
     */
    HRESULT appendToLine(size_t idxLine, const Utf8Str &rStrToAppend);

    /**
     * Prepend a string in the beginning of the given line.
     */
    HRESULT prependToLine(size_t idxLine, const Utf8Str &rStrToPrepend);

    //////////////////New functions//////////////////////////////
};


#if 0 /* convert when we fix SUSE */
/**
 * SUSE unattended XML file editor.
 */
class UnattendedSUSEXMLScript : public UnattendedXMLScript
{
public:
    UnattendedSUSEXMLScript(VirtualBoxBase *pSetError, const char *pszDefaultFilename = "autoinst.xml")
        : UnattendedXMLScript(pSetError, pszDefaultFilename) {}
    ~UnattendedSUSEXMLScript() {}

    HRESULT parse();

protected:
    HRESULT setFieldInElement(xml::ElementNode *pElement, const DataId enmDataId, const Utf8Str &rStrValue);

private:
    //////////////////New functions//////////////////////////////
    /** @throws std::bad_alloc */
    HRESULT LoopThruSections(const xml::ElementNode *pelmRoot);
    /** @throws std::bad_alloc */
    HRESULT HandleUserAccountsSection(const xml::ElementNode *pelmSection);
    /** @throws std::bad_alloc */
    Utf8Str createProbableValue(const DataId enmDataId, const xml::ElementNode *pCurElem);
    /** @throws std::bad_alloc */
    Utf8Str createProbableUserHomeDir(const xml::ElementNode *pCurElem);
    //////////////////New functions//////////////////////////////
};
#endif


#endif // !____H_UNATTENDEDSCRIPT

