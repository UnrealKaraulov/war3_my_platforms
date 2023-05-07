﻿using System;
using System.IO;
using System.Net.Sockets;
using System.Text;
using System.Text.RegularExpressions;
using System.Reflection;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using System.Globalization;
using System.Security.Authentication;
using System.Net;
using FluentFTP.Proxy;
using System.Threading;
#if !CORE
using System.Web;
#endif
#if (CORE || NETFX)
using System.Threading;

#endif
#if (CORE || NET45)
using System.Threading.Tasks;

#endif

namespace FluentFTP {
	public partial class FtpClient : IDisposable {
		#region Upload Multiple Files

		/// <summary>
		/// Uploads the given file paths to a single folder on the server.
		/// All files are placed directly into the given folder regardless of their path on the local filesystem.
		/// High-level API that takes care of various edge cases internally.
		/// Supports very large files since it uploads data in chunks.
		/// Faster than uploading single files with <see cref="o:UploadFile"/> since it performs a single "file exists" check rather than one check per file.
		/// </summary>
		/// <param name="localPaths">The full or relative paths to the files on the local file system. Files can be from multiple folders.</param>
		/// <param name="remoteDir">The full or relative path to the directory that files will be uploaded on the server</param>
		/// <param name="existsMode">What to do if the file already exists? Skip, overwrite or append? Set this to <see cref="FtpRemoteExists.NoCheck"/> for fastest performance,
		///  but only if you are SURE that the files do not exist on the server.</param>
		/// <param name="createRemoteDir">Create the remote directory if it does not exist.</param>
		/// <param name="verifyOptions">Sets if checksum verification is required for a successful download and what to do if it fails verification (See Remarks)</param>
		/// <param name="errorHandling">Used to determine how errors are handled</param>
		/// <returns>The count of how many files were uploaded successfully. Affected when files are skipped when they already exist.</returns>
		/// <remarks>
		/// If verification is enabled (All options other than <see cref="FtpVerify.None"/>) the hash will be checked against the server.  If the server does not support
		/// any hash algorithm, then verification is ignored.  If only <see cref="FtpVerify.OnlyChecksum"/> is set then the return of this method depends on both a successful 
		/// upload &amp; verification.  Additionally, if any verify option is set and a retry is attempted the existsMode will automatically be set to <see cref="FtpRemoteExists.Overwrite"/>.
		/// If <see cref="FtpVerify.Throw"/> is set and <see cref="FtpError.Throw"/> is <i>not set</i>, then individual verification errors will not cause an exception
		/// to propagate from this method.
		/// </remarks>
		public int UploadFiles(IEnumerable<string> localPaths, string remoteDir, FtpRemoteExists existsMode = FtpRemoteExists.Overwrite, bool createRemoteDir = true,
			FtpVerify verifyOptions = FtpVerify.None, FtpError errorHandling = FtpError.None) {
			// verify args
			if (!errorHandling.IsValidCombination()) {
				throw new ArgumentException("Invalid combination of FtpError flags.  Throw & Stop cannot be combined");
			}

			if (remoteDir.IsBlank()) {
				throw new ArgumentException("Required parameter is null or blank.", "remoteDir");
			}

			LogFunc("UploadFiles", new object[] { localPaths, remoteDir, existsMode, createRemoteDir, verifyOptions, errorHandling });

			//int count = 0;
			var errorEncountered = false;
			var successfulUploads = new List<string>();

			// ensure ends with slash
			remoteDir = !remoteDir.EndsWith("/") ? remoteDir + "/" : remoteDir;

			//flag to determine if existence checks are required
			var checkFileExistence = true;

			// create remote dir if wanted
			if (createRemoteDir) {
				if (!DirectoryExists(remoteDir)) {
					CreateDirectory(remoteDir);
					checkFileExistence = false;
				}
			}

			// get all the already existing files
			var existingFiles = checkFileExistence ? GetNameListing(remoteDir) : new string[0];

			// per local file
			foreach (var localPath in localPaths) {
				// calc remote path
				var fileName = Path.GetFileName(localPath);
				var remotePath = remoteDir + fileName;

				// try to upload it
				try {
					var ok = UploadFileFromFile(localPath, remotePath, false, existsMode, FtpExtensions.FileExistsInNameListing(existingFiles, remotePath), true, verifyOptions, null);
					if (ok) {
						successfulUploads.Add(remotePath);

						//count++;
					}
					else if ((int)errorHandling > 1) {
						errorEncountered = true;
						break;
					}
				}
				catch (Exception ex) {
					LogStatus(FtpTraceLevel.Error, "Upload Failure for " + localPath + ": " + ex);
					if (errorHandling.HasFlag(FtpError.Stop)) {
						errorEncountered = true;
						break;
					}

					if (errorHandling.HasFlag(FtpError.Throw)) {
						if (errorHandling.HasFlag(FtpError.DeleteProcessed)) {
							PurgeSuccessfulUploads(successfulUploads);
						}

						throw new FtpException("An error occurred uploading file(s).  See inner exception for more info.", ex);
					}
				}
			}

			if (errorEncountered) {
				//Delete any successful uploads if needed
				if (errorHandling.HasFlag(FtpError.DeleteProcessed)) {
					PurgeSuccessfulUploads(successfulUploads);
					successfulUploads.Clear(); //forces return of 0
				}

				//Throw generic error because requested
				if (errorHandling.HasFlag(FtpError.Throw)) {
					throw new FtpException("An error occurred uploading one or more files.  Refer to trace output if available.");
				}
			}

			return successfulUploads.Count;
		}

		private void PurgeSuccessfulUploads(IEnumerable<string> remotePaths) {
			foreach (var remotePath in remotePaths) {
				DeleteFile(remotePath);
			}
		}

		/// <summary>
		/// Uploads the given file paths to a single folder on the server.
		/// All files are placed directly into the given folder regardless of their path on the local filesystem.
		/// High-level API that takes care of various edge cases internally.
		/// Supports very large files since it uploads data in chunks.
		/// Faster than uploading single files with <see cref="o:UploadFile"/> since it performs a single "file exists" check rather than one check per file.
		/// </summary>
		/// <param name="localFiles">Files to be uploaded</param>
		/// <param name="remoteDir">The full or relative path to the directory that files will be uploaded on the server</param>
		/// <param name="existsMode">What to do if the file already exists? Skip, overwrite or append? Set this to FtpExists.None for fastest performance but only if you are SURE that the files do not exist on the server.</param>
		/// <param name="createRemoteDir">Create the remote directory if it does not exist.</param>
		/// <param name="verifyOptions">Sets if checksum verification is required for a successful download and what to do if it fails verification (See Remarks)</param>
		/// <param name="errorHandling">Used to determine how errors are handled</param>
		/// <returns>The count of how many files were downloaded successfully. When existing files are skipped, they are not counted.</returns>
		/// <remarks>
		/// If verification is enabled (All options other than <see cref="FtpVerify.None"/>) the hash will be checked against the server.  If the server does not support
		/// any hash algorithm, then verification is ignored.  If only <see cref="FtpVerify.OnlyChecksum"/> is set then the return of this method depends on both a successful 
		/// upload &amp; verification.  Additionally, if any verify option is set and a retry is attempted the existsMode will automatically be set to <see cref="FtpRemoteExists.Overwrite"/>.
		/// If <see cref="FtpVerify.Throw"/> is set and <see cref="FtpError.Throw"/> is <i>not set</i>, then individual verification errors will not cause an exception
		/// to propagate from this method.
		/// </remarks>
		public int UploadFiles(IEnumerable<FileInfo> localFiles, string remoteDir, FtpRemoteExists existsMode = FtpRemoteExists.Overwrite, bool createRemoteDir = true,
			FtpVerify verifyOptions = FtpVerify.None, FtpError errorHandling = FtpError.None) {
			return UploadFiles(localFiles.Select(f => f.FullName), remoteDir, existsMode, createRemoteDir, verifyOptions, errorHandling);
		}

#if ASYNC
		/// <summary>
		/// Uploads the given file paths to a single folder on the server asynchronously.
		/// All files are placed directly into the given folder regardless of their path on the local filesystem.
		/// High-level API that takes care of various edge cases internally.
		/// Supports very large files since it uploads data in chunks.
		/// Faster than uploading single files with <see cref="o:UploadFile"/> since it performs a single "file exists" check rather than one check per file.
		/// </summary>
		/// <param name="localPaths">The full or relative paths to the files on the local file system. Files can be from multiple folders.</param>
		/// <param name="remoteDir">The full or relative path to the directory that files will be uploaded on the server</param>
		/// <param name="existsMode">What to do if the file already exists? Skip, overwrite or append? Set this to FtpExists.None for fastest performance but only if you are SURE that the files do not exist on the server.</param>
		/// <param name="createRemoteDir">Create the remote directory if it does not exist.</param>
		/// <param name="verifyOptions">Sets if checksum verification is required for a successful upload and what to do if it fails verification (See Remarks)</param>
		/// <param name="errorHandling">Used to determine how errors are handled</param>
		/// <param name="token">The token to monitor for cancellation requests</param>
		/// <returns>The count of how many files were uploaded successfully. Affected when files are skipped when they already exist.</returns>
		/// <remarks>
		/// If verification is enabled (All options other than <see cref="FtpVerify.None"/>) the hash will be checked against the server.  If the server does not support
		/// any hash algorithm, then verification is ignored.  If only <see cref="FtpVerify.OnlyChecksum"/> is set then the return of this method depends on both a successful 
		/// upload &amp; verification.  Additionally, if any verify option is set and a retry is attempted the existsMode will automatically be set to <see cref="FtpRemoteExists.Overwrite"/>.
		/// If <see cref="FtpVerify.Throw"/> is set and <see cref="FtpError.Throw"/> is <i>not set</i>, then individual verification errors will not cause an exception
		/// to propagate from this method.
		/// </remarks>
		public async Task<int> UploadFilesAsync(IEnumerable<string> localPaths, string remoteDir, FtpRemoteExists existsMode = FtpRemoteExists.Overwrite, bool createRemoteDir = true, FtpVerify verifyOptions = FtpVerify.None, FtpError errorHandling = FtpError.None, CancellationToken token = default(CancellationToken)) {
			// verify args
			if (!errorHandling.IsValidCombination()) {
				throw new ArgumentException("Invalid combination of FtpError flags.  Throw & Stop cannot be combined");
			}

			if (remoteDir.IsBlank()) {
				throw new ArgumentException("Required parameter is null or blank.", "remoteDir");
			}

			LogFunc("UploadFilesAsync", new object[] { localPaths, remoteDir, existsMode, createRemoteDir, verifyOptions, errorHandling });

			//check if cancellation was requested and throw to set TaskStatus state to Canceled
			token.ThrowIfCancellationRequested();

			//int count = 0;
			var errorEncountered = false;
			var successfulUploads = new List<string>();

			// ensure ends with slash
			remoteDir = !remoteDir.EndsWith("/") ? remoteDir + "/" : remoteDir;

			//flag to determine if existence checks are required
			var checkFileExistence = true;

			// create remote dir if wanted
			if (createRemoteDir) {
				if (!await DirectoryExistsAsync(remoteDir, token)) {
					await CreateDirectoryAsync(remoteDir, token);
					checkFileExistence = false;
				}
			}

			// get all the already existing files (if directory was created just create an empty array)
			var existingFiles = checkFileExistence ? await GetNameListingAsync(remoteDir, token) : new string[0];

			// per local file
			foreach (var localPath in localPaths) {
				// check if cancellation was requested and throw to set TaskStatus state to Canceled
				token.ThrowIfCancellationRequested();

				// calc remote path
				var fileName = Path.GetFileName(localPath);
				var remotePath = remoteDir + fileName;

				// try to upload it
				try {
					bool ok = await UploadFileFromFileAsync(localPath, remotePath, false, existsMode, FtpExtensions.FileExistsInNameListing(existingFiles, remotePath), true, verifyOptions, token, null);
					if (ok) {
						successfulUploads.Add(remotePath);
					}
					else if ((int)errorHandling > 1) {
						errorEncountered = true;
						break;
					}
				}
				catch (Exception ex) {
					if (ex is OperationCanceledException) {
						//DO NOT SUPPRESS CANCELLATION REQUESTS -- BUBBLE UP!
						LogStatus(FtpTraceLevel.Info, "Upload cancellation requested");
						throw;
					}

					//suppress all other upload exceptions (errors are still written to FtpTrace)
					LogStatus(FtpTraceLevel.Error, "Upload Failure for " + localPath + ": " + ex);
					if (errorHandling.HasFlag(FtpError.Stop)) {
						errorEncountered = true;
						break;
					}

					if (errorHandling.HasFlag(FtpError.Throw)) {
						if (errorHandling.HasFlag(FtpError.DeleteProcessed)) {
							PurgeSuccessfulUploads(successfulUploads);
						}

						throw new FtpException("An error occurred uploading file(s).  See inner exception for more info.", ex);
					}
				}
			}

			if (errorEncountered) {
				//Delete any successful uploads if needed
				if (errorHandling.HasFlag(FtpError.DeleteProcessed)) {
					await PurgeSuccessfulUploadsAsync(successfulUploads);
					successfulUploads.Clear(); //forces return of 0
				}

				//Throw generic error because requested
				if (errorHandling.HasFlag(FtpError.Throw)) {
					throw new FtpException("An error occurred uploading one or more files.  Refer to trace output if available.");
				}
			}

			return successfulUploads.Count;
		}

		private async Task PurgeSuccessfulUploadsAsync(IEnumerable<string> remotePaths) {
			foreach (var remotePath in remotePaths) {
				await DeleteFileAsync(remotePath);
			}
		}
#endif

		#endregion

		#region Upload File

		/// <summary>
		/// Uploads the specified file directly onto the server.
		/// High-level API that takes care of various edge cases internally.
		/// Supports very large files since it uploads data in chunks.
		/// </summary>
		/// <param name="localPath">The full or relative path to the file on the local file system</param>
		/// <param name="remotePath">The full or relative path to the file on the server</param>
		/// <param name="existsMode">What to do if the file already exists? Skip, overwrite or append? Set this to  <see cref="FtpRemoteExists.NoCheck"/> for fastest performance 
		/// but only if you are SURE that the files do not exist on the server.</param>
		/// <param name="createRemoteDir">Create the remote directory if it does not exist. Slows down upload due to additional checks required.</param>
		/// <param name="verifyOptions">Sets if checksum verification is required for a successful upload and what to do if it fails verification (See Remarks)</param>
		/// <param name="progress">Provide an implementation of IProgress to track upload progress. The value provided is in the range 0 to 100, indicating the percentage of the file transferred. If the progress is indeterminate, -1 is sent.</param>
		/// <returns>If true then the file was uploaded, false otherwise.</returns>
		/// <remarks>
		/// If verification is enabled (All options other than <see cref="FtpVerify.None"/>) the hash will be checked against the server.  If the server does not support
		/// any hash algorithm, then verification is ignored.  If only <see cref="FtpVerify.OnlyChecksum"/> is set then the return of this method depends on both a successful 
		/// upload &amp; verification.  Additionally, if any verify option is set and a retry is attempted the existsMode will automatically be set to <see cref="FtpRemoteExists.Overwrite"/>.
		/// </remarks>
		public bool UploadFile(string localPath, string remotePath, FtpRemoteExists existsMode = FtpRemoteExists.Overwrite, bool createRemoteDir = false,
			FtpVerify verifyOptions = FtpVerify.None, Action<FtpProgress> progress = null) {
			// verify args
			if (localPath.IsBlank()) {
				throw new ArgumentException("Required parameter is null or blank.", "localPath");
			}

			if (remotePath.IsBlank()) {
				throw new ArgumentException("Required parameter is null or blank.", "remotePath");
			}

			LogFunc("UploadFile", new object[] { localPath, remotePath, existsMode, createRemoteDir, verifyOptions });

			// skip uploading if the local file does not exist
			if (!File.Exists(localPath)) {
				LogStatus(FtpTraceLevel.Error, "File does not exist.");
				return false;
			}

			return UploadFileFromFile(localPath, remotePath, createRemoteDir, existsMode, false, false, verifyOptions, progress);
		}

		private bool UploadFileFromFile(string localPath, string remotePath, bool createRemoteDir, FtpRemoteExists existsMode, bool fileExists, bool fileExistsKnown, FtpVerify verifyOptions, Action<FtpProgress> progress) {
			// If retries are allowed set the retry counter to the allowed count
			var attemptsLeft = verifyOptions.HasFlag(FtpVerify.Retry) ? m_retryAttempts : 1;

			// Default validation to true (if verification isn't needed it'll allow a pass-through)
			var verified = true;
			bool uploadSuccess;
			do {
				// write the file onto the server
				using (var fileStream = new FileStream(localPath, FileMode.Open, FileAccess.Read, FileShare.Read)) {
					// Upload file
					uploadSuccess = UploadFileInternal(fileStream, remotePath, createRemoteDir, existsMode, fileExists, fileExistsKnown, progress);
					attemptsLeft--;

					// If verification is needed, update the validated flag
					if (uploadSuccess && verifyOptions != FtpVerify.None) {
						verified = VerifyTransfer(localPath, remotePath);
						LogStatus(FtpTraceLevel.Info, "File Verification: " + (verified ? "PASS" : "FAIL"));
						if (!verified && attemptsLeft > 0) {
							// Force overwrite if a retry is required
							LogStatus(FtpTraceLevel.Verbose, "Retrying due to failed verification." + (existsMode != FtpRemoteExists.Overwrite ? "  Switching to FtpExists.Overwrite mode.  " : "  ") + attemptsLeft + " attempts remaining");
							existsMode = FtpRemoteExists.Overwrite;
						}
					}
				}
			} while (!verified && attemptsLeft > 0); //Loop if attempts are available and validation failed


			if (uploadSuccess && !verified && verifyOptions.HasFlag(FtpVerify.Delete)) {
				DeleteFile(remotePath);
			}

			if (uploadSuccess && !verified && verifyOptions.HasFlag(FtpVerify.Throw)) {
				throw new FtpException("Uploaded file checksum value does not match local file");
			}

			return uploadSuccess && verified;
		}

#if ASYNC
		/// <summary>
		/// Uploads the specified file directly onto the server asynchronously.
		/// High-level API that takes care of various edge cases internally.
		/// Supports very large files since it uploads data in chunks.
		/// </summary>
		/// <param name="localPath">The full or relative path to the file on the local file system</param>
		/// <param name="remotePath">The full or relative path to the file on the server</param>
		/// <param name="existsMode">What to do if the file already exists? Skip, overwrite or append? Set this to  <see cref="FtpRemoteExists.NoCheck"/> for fastest performance
		///  but only if you are SURE that the files do not exist on the server.</param>
		/// <param name="createRemoteDir">Create the remote directory if it does not exist. Slows down upload due to additional checks required.</param>
		/// <param name="verifyOptions">Sets if checksum verification is required for a successful upload and what to do if it fails verification (See Remarks)</param>
		/// <param name="token">The token to monitor for cancellation requests.</param>
		/// <param name="progress">Provide an implementation of IProgress to track upload progress. The value provided is in the range 0 to 100, indicating the percentage of the file transferred. If the progress is indeterminate, -1 is sent.</param>
		/// <returns>If true then the file was uploaded, false otherwise.</returns>
		/// <remarks>
		/// If verification is enabled (All options other than <see cref="FtpVerify.None"/>) the hash will be checked against the server.  If the server does not support
		/// any hash algorithm, then verification is ignored.  If only <see cref="FtpVerify.OnlyChecksum"/> is set then the return of this method depends on both a successful 
		/// upload &amp; verification.  Additionally, if any verify option is set and a retry is attempted the existsMode will automatically be set to <see cref="FtpRemoteExists.Overwrite"/>.
		/// </remarks>
		public async Task<bool> UploadFileAsync(string localPath, string remotePath, FtpRemoteExists existsMode = FtpRemoteExists.Overwrite, bool createRemoteDir = false, FtpVerify verifyOptions = FtpVerify.None, IProgress<FtpProgress> progress = null, CancellationToken token = default(CancellationToken)) {
			// verify args
			if (localPath.IsBlank()) {
				throw new ArgumentException("Required parameter is null or blank.", "localPath");
			}

			if (remotePath.IsBlank()) {
				throw new ArgumentException("Required parameter is null or blank.", "remotePath");
			}

			// skip uploading if the local file does not exist
#if CORE
			if (!await Task.Run(() => File.Exists(localPath), token)) {
#else
			if (!File.Exists(localPath)) {
#endif
				LogStatus(FtpTraceLevel.Error, "File does not exist.");
				return false;
			}

			LogFunc("UploadFileAsync", new object[] { localPath, remotePath, existsMode, createRemoteDir, verifyOptions });

			return await UploadFileFromFileAsync(localPath, remotePath, createRemoteDir, existsMode, false, false, verifyOptions, token, progress);
		}

		private async Task<bool> UploadFileFromFileAsync(string localPath, string remotePath, bool createRemoteDir, FtpRemoteExists existsMode,
			bool fileExists, bool fileExistsKnown, FtpVerify verifyOptions, CancellationToken token, IProgress<FtpProgress> progress) {
			// If retries are allowed set the retry counter to the allowed count
			var attemptsLeft = verifyOptions.HasFlag(FtpVerify.Retry) ? m_retryAttempts : 1;

			// Default validation to true (if verification isn't needed it'll allow a pass-through)
			var verified = true;
			bool uploadSuccess;
			do {
				// write the file onto the server
				using (var fileStream = new FileStream(localPath, FileMode.Open, FileAccess.Read, FileShare.Read, 4096, true)) {
					uploadSuccess = await UploadFileInternalAsync(fileStream, remotePath, createRemoteDir, existsMode, fileExists, fileExistsKnown, progress, token);
					attemptsLeft--;

					// If verification is needed, update the validated flag
					if (verifyOptions != FtpVerify.None) {
						verified = await VerifyTransferAsync(localPath, remotePath, token);
						LogStatus(FtpTraceLevel.Info, "File Verification: " + (verified ? "PASS" : "FAIL"));
						if (!verified && attemptsLeft > 0) {
							// Force overwrite if a retry is required
							LogStatus(FtpTraceLevel.Verbose, "Retrying due to failed verification." + (existsMode != FtpRemoteExists.Overwrite ? "  Switching to FtpExists.Overwrite mode.  " : "  ") + attemptsLeft + " attempts remaining");
							existsMode = FtpRemoteExists.Overwrite;
						}
					}
				}
			} while (!verified && attemptsLeft > 0);

			if (uploadSuccess && !verified && verifyOptions.HasFlag(FtpVerify.Delete)) {
				await DeleteFileAsync(remotePath, token);
			}

			if (uploadSuccess && !verified && verifyOptions.HasFlag(FtpVerify.Throw)) {
				throw new FtpException("Uploaded file checksum value does not match local file");
			}

			return uploadSuccess && verified;
		}

#endif

		#endregion

		#region	Upload Bytes/Stream

		/// <summary>
		/// Uploads the specified stream as a file onto the server.
		/// High-level API that takes care of various edge cases internally.
		/// Supports very large files since it uploads data in chunks.
		/// </summary>
		/// <param name="fileStream">The full data of the file, as a stream</param>
		/// <param name="remotePath">The full or relative path to the file on the server</param>
		/// <param name="existsMode">What to do if the file already exists? Skip, overwrite or append? Set this to <see cref="FtpRemoteExists.NoCheck"/> for fastest performance
		/// but only if you are SURE that the files do not exist on the server.</param>
		/// <param name="createRemoteDir">Create the remote directory if it does not exist. Slows down upload due to additional checks required.</param>
		/// <param name="progress">Provide an implementation of IProgress to track upload progress. The value provided is in the range 0 to 100, indicating the percentage of the file transferred. If the progress is indeterminate, -1 is sent.</param>
		public bool Upload(Stream fileStream, string remotePath, FtpRemoteExists existsMode = FtpRemoteExists.Overwrite, bool createRemoteDir = false, Action<FtpProgress> progress = null) {
			// verify args
			if (fileStream == null) {
				throw new ArgumentException("Required parameter is null or blank.", "fileStream");
			}

			if (remotePath.IsBlank()) {
				throw new ArgumentException("Required parameter is null or blank.", "remotePath");
			}

			LogFunc("Upload", new object[] { remotePath, existsMode, createRemoteDir });

			// write the file onto the server
			return UploadFileInternal(fileStream, remotePath, createRemoteDir, existsMode, false, false, progress);
		}

		/// <summary>
		/// Uploads the specified byte array as a file onto the server.
		/// High-level API that takes care of various edge cases internally.
		/// Supports very large files since it uploads data in chunks.
		/// </summary>
		/// <param name="fileData">The full data of the file, as a byte array</param>
		/// <param name="remotePath">The full or relative path to the file on the server</param>
		/// <param name="existsMode">What to do if the file already exists? Skip, overwrite or append? Set this to <see cref="FtpRemoteExists.NoCheck"/> for fastest performance 
		/// but only if you are SURE that the files do not exist on the server.</param>
		/// <param name="createRemoteDir">Create the remote directory if it does not exist. Slows down upload due to additional checks required.</param>
		/// <param name="progress">Provide an implementation of IProgress to track upload progress. The value provided is in the range 0 to 100, indicating the percentage of the file transferred. If the progress is indeterminate, -1 is sent.</param>
		public bool Upload(byte[] fileData, string remotePath, FtpRemoteExists existsMode = FtpRemoteExists.Overwrite, bool createRemoteDir = false, Action<FtpProgress> progress = null) {
			// verify args
			if (fileData == null) {
				throw new ArgumentException("Required parameter is null or blank.", "fileData");
			}

			if (remotePath.IsBlank()) {
				throw new ArgumentException("Required parameter is null or blank.", "remotePath");
			}

			LogFunc("Upload", new object[] { remotePath, existsMode, createRemoteDir });

			// write the file onto the server
			using (var ms = new MemoryStream(fileData)) {
				ms.Position = 0;
				return UploadFileInternal(ms, remotePath, createRemoteDir, existsMode, false, false, progress);
			}
		}


#if ASYNC
		/// <summary>
		/// Uploads the specified stream as a file onto the server asynchronously.
		/// High-level API that takes care of various edge cases internally.
		/// Supports very large files since it uploads data in chunks.
		/// </summary>
		/// <param name="fileStream">The full data of the file, as a stream</param>
		/// <param name="remotePath">The full or relative path to the file on the server</param>
		/// <param name="existsMode">What to do if the file already exists? Skip, overwrite or append? Set this to <see cref="FtpRemoteExists.NoCheck"/> for fastest performance,
		///  but only if you are SURE that the files do not exist on the server.</param>
		/// <param name="createRemoteDir">Create the remote directory if it does not exist. Slows down upload due to additional checks required.</param>
		/// <param name="token">The token to monitor for cancellation requests.</param>
		/// <param name="progress">Provide an implementation of IProgress to track upload progress. The value provided is in the range 0 to 100, indicating the percentage of the file transferred. If the progress is indeterminate, -1 is sent.</param>
		/// <returns>If true then the file was uploaded, false otherwise.</returns>
		public async Task<bool> UploadAsync(Stream fileStream, string remotePath, FtpRemoteExists existsMode = FtpRemoteExists.Overwrite, bool createRemoteDir = false, IProgress<FtpProgress> progress = null, CancellationToken token = default(CancellationToken)) {
			// verify args
			if (fileStream == null) {
				throw new ArgumentException("Required parameter is null or blank.", "fileStream");
			}

			if (remotePath.IsBlank()) {
				throw new ArgumentException("Required parameter is null or blank.", "remotePath");
			}

			LogFunc("UploadAsync", new object[] { remotePath, existsMode, createRemoteDir });

			// write the file onto the server
			return await UploadFileInternalAsync(fileStream, remotePath, createRemoteDir, existsMode, false, false, progress, token);
		}

		/// <summary>
		/// Uploads the specified byte array as a file onto the server asynchronously.
		/// High-level API that takes care of various edge cases internally.
		/// Supports very large files since it uploads data in chunks.
		/// </summary>
		/// <param name="fileData">The full data of the file, as a byte array</param>
		/// <param name="remotePath">The full or relative path to the file on the server</param>
		/// <param name="existsMode">What to do if the file already exists? Skip, overwrite or append? Set this to <see cref="FtpRemoteExists.NoCheck"/> for fastest performance,
		///  but only if you are SURE that the files do not exist on the server.</param>
		/// <param name="createRemoteDir">Create the remote directory if it does not exist. Slows down upload due to additional checks required.</param>
		/// <param name="token">The token to monitor for cancellation requests.</param>
		/// <param name="progress">Provide an implementation of IProgress to track upload progress. The value provided is in the range 0 to 100, indicating the percentage of the file transferred. If the progress is indeterminate, -1 is sent.</param>
		/// <returns>If true then the file was uploaded, false otherwise.</returns>
		public async Task<bool> UploadAsync(byte[] fileData, string remotePath, FtpRemoteExists existsMode = FtpRemoteExists.Overwrite, bool createRemoteDir = false, IProgress<FtpProgress> progress = null, CancellationToken token = default(CancellationToken)) {
			// verify args
			if (fileData == null) {
				throw new ArgumentException("Required parameter is null or blank.", "fileData");
			}

			if (remotePath.IsBlank()) {
				throw new ArgumentException("Required parameter is null or blank.", "remotePath");
			}

			LogFunc("UploadAsync", new object[] { remotePath, existsMode, createRemoteDir });

			// write the file onto the server
			using (var ms = new MemoryStream(fileData)) {
				ms.Position = 0;
				return await UploadFileInternalAsync(ms, remotePath, createRemoteDir, existsMode, false, false, progress, token);
			}
		}
#endif

		#endregion

		#region Upload File Internal

		/// <summary>
		/// Upload the given stream to the server as a new file. Overwrites the file if it exists.
		/// Writes data in chunks. Retries if server disconnects midway.
		/// </summary>
		private bool UploadFileInternal(Stream fileData, string remotePath, bool createRemoteDir, FtpRemoteExists existsMode, bool fileExists, bool fileExistsKnown, Action<FtpProgress> progress) {
			Stream upStream = null;

			try {
				long offset = 0;
				var checkFileExistsAgain = false;

				// check if the file exists, and skip, overwrite or append
				if (existsMode == FtpRemoteExists.NoCheck) {
					checkFileExistsAgain = false;
				}
				else if (existsMode == FtpRemoteExists.AppendNoCheck) {
					checkFileExistsAgain = true;

					offset = GetFileSize(remotePath);
					if (offset == -1) {
						offset = 0; // start from the beginning
					}
				}
				else {
					if (!fileExistsKnown) {
						fileExists = FileExists(remotePath);
					}

					switch (existsMode) {
						case FtpRemoteExists.Skip:
							if (fileExists) {
								LogStatus(FtpTraceLevel.Warn, "File " + remotePath + " exists on server & existsMode is set to FileExists.Skip");

								// Fix #413 - progress callback isn't called if the file has already been uploaded to the server
								// send progress reports
								if (progress != null) {
									progress(new FtpProgress(100.0, 0, TimeSpan.FromSeconds(0)));
								}

								return false;
							}

							break;

						case FtpRemoteExists.Overwrite:
							if (fileExists) {
								DeleteFile(remotePath);
							}

							break;

						case FtpRemoteExists.Append:
							if (fileExists) {
								offset = GetFileSize(remotePath);
								if (offset == -1) {
									offset = 0; // start from the beginning
								}
							}

							break;
					}
				}

				// ensure the remote dir exists .. only if the file does not already exist!
				if (createRemoteDir && !fileExists) {
					var dirname = remotePath.GetFtpDirectoryName();
					if (!DirectoryExists(dirname)) {
						CreateDirectory(dirname);
					}
				}

				// FIX #213 : Do not change Stream.Position if not supported
				if (fileData.CanSeek) {
					try {
						// seek to required offset
						fileData.Position = offset;
					}
					catch (Exception ex2) {
					}
				}

				// open a file connection
				if (offset == 0 && existsMode != FtpRemoteExists.AppendNoCheck) {
					upStream = OpenWrite(remotePath, UploadDataType, checkFileExistsAgain);
				}
				else {
					upStream = OpenAppend(remotePath, UploadDataType, checkFileExistsAgain);
				}

				const int rateControlResolution = 100;
				var rateLimitBytes = UploadRateLimit != 0 ? (long)UploadRateLimit * 1024 : 0;
				var chunkSize = TransferChunkSize;
				if (m_transferChunkSize == null && rateLimitBytes > 0) {
					// reduce chunk size to optimize rate control
					const int chunkSizeMin = 64;
					while (chunkSize > chunkSizeMin) {
						var chunkLenInMs = 1000L * chunkSize / rateLimitBytes;
						if (chunkLenInMs <= rateControlResolution) {
							break;
						}

						chunkSize = Math.Max(chunkSize >> 1, chunkSizeMin);
					}
				}

				// loop till entire file uploaded
				var fileLen = fileData.Length;
				var buffer = new byte[chunkSize];

				var transferStarted = DateTime.Now;
				var sw = new Stopwatch();

				// Fix #288 - Upload hangs with only a few bytes left
				if (fileLen < upStream.Length) {
					upStream.SetLength(fileLen);
				}

				var anyNoop = false;

				while (offset < fileLen) {
					try {
						// read a chunk of bytes from the file
						int readBytes;
						long limitCheckBytes = 0;
						long bytesProcessed = 0;

						sw.Start();
						while ((readBytes = fileData.Read(buffer, 0, buffer.Length)) > 0) {
							// write chunk to the FTP stream
							upStream.Write(buffer, 0, readBytes);
							upStream.Flush();
							offset += readBytes;
							bytesProcessed += readBytes;
							limitCheckBytes += readBytes;

							// send progress reports
							if (progress != null) {
								ReportProgress(progress, fileLen, offset, bytesProcessed, DateTime.Now - transferStarted);
							}

							// Fix #387: keep alive with NOOP as configured and needed
							if (!m_threadSafeDataChannels) {
								anyNoop = Noop() || anyNoop;
							}

							// honor the speed limit
							var swTime = sw.ElapsedMilliseconds;
							if (rateLimitBytes > 0) {
								var timeShouldTake = limitCheckBytes * 1000 / rateLimitBytes;
								if (timeShouldTake > swTime) {
#if CORE14
										Task.Delay((int) (timeShouldTake - swTime)).Wait();
#else
									Thread.Sleep((int)(timeShouldTake - swTime));
#endif
								}
								else if (swTime > timeShouldTake + rateControlResolution) {
									limitCheckBytes = 0;
									sw.Restart();
								}
							}
						}

						// zero return value (with no Exception) indicates EOS; so we should terminate the outer loop here
						break;
					}
					catch (IOException ex) {
						// resume if server disconnected midway, or throw if there is an exception doing that as well
						if (!ResumeUpload(remotePath, ref upStream, offset, ex)) {
							sw.Stop();
							throw;
						}
					}
					catch (TimeoutException ex) {
						// fix: attempting to upload data after we reached the end of the stream
						// often throws a timeout execption, so we silently absorb that here
						if (offset >= fileLen) {
							break;
						}
						else {
							sw.Stop();
							throw;
						}
					}
				}

				sw.Stop();


				// wait for transfer to get over
				while (upStream.Position < upStream.Length) {
				}

				// send progress reports
				if (progress != null) {
					progress(new FtpProgress(100.0, 0, TimeSpan.FromSeconds(0)));
				}

				// disconnect FTP stream before exiting
				upStream.Dispose();

				// FIX : if this is not added, there appears to be "stale data" on the socket
				// listen for a success/failure reply
				try {
					while (!m_threadSafeDataChannels) {
						var status = GetReply();

						// Fix #387: exhaust any NOOP responses (not guaranteed during file transfers)
						if (anyNoop && status.Message != null && status.Message.Contains("NOOP")) {
							continue;
						}

						// Fix #353: if server sends 550 the transfer was received but could not be confirmed by the server
						if (status.Code != null && status.Code != "" && status.Code.StartsWith("5")) {
							return false;
						}

						// Fix #387: exhaust any NOOP responses also after "226 Transfer complete."
						if (anyNoop) {
							ReadStaleData(false, true, true);
						}

						break;
					}
				}

				// absorb "System.TimeoutException: Timed out trying to read data from the socket stream!" at GetReply()
				catch (Exception) { }

				return true;
			}
			catch (Exception ex1) {
				// close stream before throwing error
				try {
					if (upStream != null) {
						upStream.Dispose();
					}
				}
				catch (Exception) {
				}

				// catch errors during upload
				throw new FtpException("Error while uploading the file to the server. See InnerException for more info.", ex1);
			}
		}

#if ASYNC
		/// <summary>
		/// Upload the given stream to the server as a new file asynchronously. Overwrites the file if it exists.
		/// Writes data in chunks. Retries if server disconnects midway.
		/// </summary>
		private async Task<bool> UploadFileInternalAsync(Stream fileData, string remotePath, bool createRemoteDir, FtpRemoteExists existsMode, bool fileExists, bool fileExistsKnown, IProgress<FtpProgress> progress, CancellationToken token = default(CancellationToken)) {
			Stream upStream = null;
			try {
				long offset = 0;
				var checkFileExistsAgain = false;

				// check if the file exists, and skip, overwrite or append
				if (existsMode == FtpRemoteExists.NoCheck) {
					checkFileExistsAgain = false;
				}
				else if (existsMode == FtpRemoteExists.AppendNoCheck) {
					checkFileExistsAgain = true;
					offset = await GetFileSizeAsync(remotePath, token);
					if (offset == -1) {
						offset = 0; // start from the beginning
					}
				}
				else {
					if (!fileExistsKnown) {
						fileExists = await FileExistsAsync(remotePath, token);
					}

					switch (existsMode) {
						case FtpRemoteExists.Skip:
							if (fileExists) {
								LogStatus(FtpTraceLevel.Warn, "File " + remotePath + " exists on server & existsMode is set to FileExists.Skip");

								// Fix #413 - progress callback isn't called if the file has already been uploaded to the server
								// send progress reports
								if (progress != null) {
									progress.Report(new FtpProgress(100.0, 0, TimeSpan.FromSeconds(0)));
								}

								return false;
							}

							break;

						case FtpRemoteExists.Overwrite:
							if (fileExists) {
								await DeleteFileAsync(remotePath, token);
							}

							break;

						case FtpRemoteExists.Append:
							if (fileExists) {
								offset = await GetFileSizeAsync(remotePath, token);
								if (offset == -1) {
									offset = 0; // start from the beginning
								}
							}

							break;
					}
				}

				// ensure the remote dir exists .. only if the file does not already exist!
				if (createRemoteDir && !fileExists) {
					var dirname = remotePath.GetFtpDirectoryName();
					if (!await DirectoryExistsAsync(dirname, token)) {
						await CreateDirectoryAsync(dirname, token);
					}
				}

				// FIX #213 : Do not change Stream.Position if not supported
				if (fileData.CanSeek) {
					try {
						// seek to required offset
						fileData.Position = offset;
					}
					catch (Exception ex2) {
					}
				}

				// open a file connection
				if (offset == 0 && existsMode != FtpRemoteExists.AppendNoCheck) {
					upStream = await OpenWriteAsync(remotePath, UploadDataType, checkFileExistsAgain, token);
				}
				else {
					upStream = await OpenAppendAsync(remotePath, UploadDataType, checkFileExistsAgain, token);
				}

				const int rateControlResolution = 100;
				var rateLimitBytes = UploadRateLimit != 0 ? (long)UploadRateLimit * 1024 : 0;
				var chunkSize = TransferChunkSize;
				if (m_transferChunkSize == null && rateLimitBytes > 0) {
					// reduce chunk size to optimize rate control
					const int chunkSizeMin = 64;
					while (chunkSize > chunkSizeMin) {
						var chunkLenInMs = 1000L * chunkSize / rateLimitBytes;
						if (chunkLenInMs <= rateControlResolution) {
							break;
						}
						chunkSize = Math.Max(chunkSize >> 1, chunkSizeMin);
					}
				}

				// loop till entire file uploaded
				var fileLen = fileData.Length;
				var buffer = new byte[chunkSize];

				var transferStarted = DateTime.Now;
				var sw = new Stopwatch();

				// Fix #288 - Upload hangs with only a few bytes left
				if (fileLen < upStream.Length) {
					upStream.SetLength(fileLen);
				}

				var anyNoop = false;

				while (offset < fileLen) {
					try {
						// read a chunk of bytes from the file
						int readBytes;
						long limitCheckBytes = 0;
						long bytesProcessed = 0;

						sw.Start();
						while ((readBytes = await fileData.ReadAsync(buffer, 0, buffer.Length, token)) > 0) {
							// write chunk to the FTP stream
							await upStream.WriteAsync(buffer, 0, readBytes, token);
							await upStream.FlushAsync(token);
							offset += readBytes;
							bytesProcessed += readBytes;
							limitCheckBytes += readBytes;

							// send progress reports
							if (progress != null) {
								ReportProgress(progress, fileLen, offset, bytesProcessed, DateTime.Now - transferStarted);
							}

							// Fix #387: keep alive with NOOP as configured and needed
							if (!m_threadSafeDataChannels) {
								anyNoop = await NoopAsync(token) || anyNoop;
							}

							// honor the rate limit
							var swTime = sw.ElapsedMilliseconds;
							if (rateLimitBytes > 0) {
								var timeShouldTake = limitCheckBytes * 1000 / rateLimitBytes;
								if (timeShouldTake > swTime) {
									await Task.Delay((int)(timeShouldTake - swTime), token);
									token.ThrowIfCancellationRequested();
								}
								else if (swTime > timeShouldTake + rateControlResolution) {
									limitCheckBytes = 0;
									sw.Restart();
								}
							}
						}

						// zero return value (with no Exception) indicates EOS; so we should terminate the outer loop here
						break;
					}
					catch (IOException ex) {
						// resume if server disconnected midway, or throw if there is an exception doing that as well
						var resumeResult = await ResumeUploadAsync(remotePath, upStream, offset, ex);
						if (resumeResult.Item1) {
							upStream = resumeResult.Item2;
						}
						else {
							sw.Stop();
							throw;
						}
					}
					catch (TimeoutException ex) {
						// fix: attempting to upload data after we reached the end of the stream
						// often throws a timeout execption, so we silently absorb that here
						if (offset >= fileLen) {
							break;
						}
						else {
							sw.Stop();
							throw;
						}
					}
				}

				sw.Stop();

				// wait for transfer to get over
				while (upStream.Position < upStream.Length) {
				}

				// send progress reports
				if (progress != null) {
					progress.Report(new FtpProgress(100.0, 0, TimeSpan.FromSeconds(0)));
				}

				// disconnect FTP stream before exiting
				upStream.Dispose();

				// FIX : if this is not added, there appears to be "stale data" on the socket
				// listen for a success/failure reply
				try {
					while (!m_threadSafeDataChannels) {
						FtpReply status = await GetReplyAsync(token);

						// Fix #387: exhaust any NOOP responses (not guaranteed during file transfers)
						if (anyNoop && status.Message != null && status.Message.Contains("NOOP")) {
							continue;
						}

						// Fix #353: if server sends 550 the transfer was received but could not be confirmed by the server
						if (status.Code != null && status.Code != "" && status.Code.StartsWith("5")) {
							return false;
						}

						// Fix #387: exhaust any NOOP responses also after "226 Transfer complete."
						if (anyNoop) {
							await ReadStaleDataAsync(false, true, true, token);
						}

						break;
					}
				}

				// absorb "System.TimeoutException: Timed out trying to read data from the socket stream!" at GetReply()
				catch (Exception) { }

				return true;
			}
			catch (Exception ex1) {
				// close stream before throwing error
				try {
					if (upStream != null) {
						upStream.Dispose();
					}
				}
				catch (Exception) {
				}

				if (ex1 is OperationCanceledException) {
					LogStatus(FtpTraceLevel.Info, "Upload cancellation requested");
					throw;
				}

				// catch errors during upload
				throw new FtpException("Error while uploading the file to the server. See InnerException for more info.", ex1);
			}
		}

#endif

		private bool ResumeUpload(string remotePath, ref Stream upStream, long offset, IOException ex) {
			// resume if server disconnects midway (fixes #39)
			if (ex.InnerException != null) {
				var iex = ex.InnerException as SocketException;
#if CORE
					if (iex != null && (int) iex.SocketErrorCode == 10054) {
#else
				if (iex != null && iex.ErrorCode == 10054) {
#endif
					upStream.Dispose();
					upStream = OpenAppend(remotePath, UploadDataType, true);
					upStream.Position = offset;
					return true;
				}
			}

			return false;
		}

#if ASYNC
		private async Task<Tuple<bool, Stream>> ResumeUploadAsync(string remotePath, Stream upStream, long offset, IOException ex) {
			// resume if server disconnects midway (fixes #39)
			if (ex.InnerException != null) {
				var iex = ex.InnerException as SocketException;
#if CORE
				if (iex != null && (int) iex.SocketErrorCode == 10054) {
#else
				if (iex != null && iex.ErrorCode == 10054) {
#endif
					upStream.Dispose();
					var returnStream = await OpenAppendAsync(remotePath, UploadDataType, true);
					returnStream.Position = offset;
					return Tuple.Create(true, returnStream);
				}
			}

			return Tuple.Create(false, (Stream)null);
		}
#endif

		#endregion
	}
}