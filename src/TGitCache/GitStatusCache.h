// TortoiseSVN - a Windows shell extension for easy version control

// External Cache Copyright (C) 2005 - 2006 - Will Dean, Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#pragma once

#include "TGitPath.h"
//#include "SVNHelpers.h"
#include "StatusCacheEntry.h"
#include "CachedDirectory.h"
#include "FolderCrawler.h"
#include "DirectoryWatcher.h"
#include "ShellUpdater.h"
#include "RWSection.h"
#include "atlcoll.h"

//////////////////////////////////////////////////////////////////////////

/**
 * \ingroup TSVNCache
 * The main class handling the status cache.
 * Provides access to a global object of itself which handles all
 * the requests for status.
 */
class CGitStatusCache
{
private:
	CGitStatusCache(void);
	~CGitStatusCache(void);

public:
	static CGitStatusCache& Instance();
	static void Create();
	static void Destroy();
	static bool SaveCache();

public:
	GitStatus m_GitStatus;
	/// Refreshes the cache.
	void Refresh();

	/// Get the status for a single path (main entry point, called from named-pipe code
	CStatusCacheEntry GetStatusForPath(const CTGitPath& path, DWORD flags,  bool bFetch = true);

	/// Find a directory in the cache (a new entry will be created if there isn't an existing entry)
	CCachedDirectory * GetDirectoryCacheEntry(const CTGitPath& path);
	CCachedDirectory * GetDirectoryCacheEntryNoCreate(const CTGitPath& path);

	/// Add a folder to the background crawler's work list
	void AddFolderForCrawling(const CTGitPath& path);

	/// Removes the cache for a specific path, e.g. if a folder got deleted/renamed
	void RemoveCacheForPath(const CTGitPath& path);

	/// Removes all items from the cache
	void ClearCache();
	
	/// Call this method before getting the status for a shell request
	void StartRequest(const CTGitPath& path);
	/// Call this method after the data for the shell request has been gathered
	void EndRequest(const CTGitPath& path);
	
	/// Notifies the shell about file/folder status changes.
	/// A notification is only sent for paths which aren't currently
	/// in the list of handled shell requests to avoid deadlocks.
	void UpdateShell(const CTGitPath& path);

	size_t GetCacheSize() {return m_directoryCache.size();}
	int GetNumberOfWatchedPaths() {return watcher.GetNumberOfWatchedPaths();}

	void Init();
	void Stop();

	void CloseWatcherHandles(HDEVNOTIFY hdev);
	void CGitStatusCache::CloseWatcherHandles(const CTGitPath& path);

	bool WaitToRead(DWORD waitTime = INFINITE) {return m_rwSection.WaitToRead(waitTime);}
	bool WaitToWrite(DWORD waitTime = INFINITE) {return m_rwSection.WaitToWrite(waitTime);}
	void Done() {m_rwSection.Done();}
	bool IsWriter() {return m_rwSection.IsWriter();}
#if defined (DEBUG) || defined (_DEBUG)
	void AssertLock() {m_rwSection.AssertLock();}
	void AssertWriting() {m_rwSection.AssertWriting();}
#else
	void AssertLock() {;}
	void AssertWriting() {;}
#endif
	bool IsPathAllowed(const CTGitPath& path) {return !!m_shellCache.IsPathAllowed(path.GetWinPath());}
	bool IsUnversionedAsModified() {return !!m_shellCache.IsUnversionedAsModified();}
	bool IsPathGood(const CTGitPath& path);
	bool IsPathWatched(const CTGitPath& path) {return watcher.IsPathWatched(path);}
	bool AddPathToWatch(const CTGitPath& path) {return watcher.AddPath(path);}

	bool m_bClearMemory;
private:
	bool RemoveCacheForDirectory(CCachedDirectory * cdir);
	CRWSection m_rwSection;
	CAtlList<CString> m_askedList;
	CCachedDirectory::CachedDirMap m_directoryCache;
	std::set<CTGitPath> m_NoWatchPaths;
//	SVNHelper m_svnHelp;
	ShellCache	m_shellCache;

	static CGitStatusCache* m_pInstance;

	CFolderCrawler m_folderCrawler;
	CShellUpdater m_shellUpdater;

	CTGitPath m_mostRecentPath;
	CStatusCacheEntry m_mostRecentStatus;
	long m_mostRecentExpiresAt;

	CDirectoryWatcher watcher;

	friend class CCachedDirectory;  // Needed for access to the SVN helpers
};
