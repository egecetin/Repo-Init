import json
import argparse
import logging
from datetime import datetime
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor, as_completed
from tqdm import tqdm
import requests

class BatchCrashpadUploader:
    def __init__(self, dsn, org_slug=None, project_slug=None, auth_token=None):
        """
        Initialize the batch uploader
        
        Args:
            dsn: Sentry DSN
            org_slug: Organization slug (optional, for API uploads)
            project_slug: Project slug (optional, for API uploads)
            auth_token: Auth token (optional, for API uploads)
        """
        self.dsn = dsn
        self.org_slug = org_slug
        self.project_slug = project_slug
        self.auth_token = auth_token
        self.setup_logging()
        
    def setup_logging(self, log_level=logging.INFO):
        """Setup logging configuration"""
        logging.basicConfig(
            level=log_level,
            format='%(asctime)s - %(levelname)s - %(message)s',
            handlers=[
                logging.FileHandler('crashpad_upload.log'),
                logging.StreamHandler()
            ]
        )
        self.logger = logging.getLogger(__name__)
        
    def validate_directory(self, reports_directory):
        """Validate the reports directory exists and contains minidump files"""
        reports_dir = Path(reports_directory)
        
        if not reports_dir.exists():
            raise FileNotFoundError(f"Directory {reports_directory} does not exist")
            
        if not reports_dir.is_dir():
            raise NotADirectoryError(f"{reports_directory} is not a directory")
            
        minidump_files = list(reports_dir.glob("*.dmp"))
        if not minidump_files:
            self.logger.warning(f"No .dmp files found in {reports_directory}")
            return []
            
        self.logger.info(f"Found {len(minidump_files)} minidump files to upload")
        return minidump_files
        
    def upload_single_minidump(self, minidump_path):
        """Upload a single minidump file to Sentry"""
        minidump_path = Path(minidump_path)
        
        # Get file timestamp to preserve original crash time
        file_stat = minidump_path.stat()
        crash_timestamp = datetime.fromtimestamp(file_stat.st_mtime)
        
        # Look for associated metadata file
        metadata_file = minidump_path.with_suffix('.meta')
        metadata = {}
        
        if metadata_file.exists():
            try:
                # Try to read as JSON first
                with open(metadata_file, 'r', encoding='utf-8') as f:
                    metadata = json.load(f)
                self.logger.debug(f"Loaded JSON metadata for {minidump_path.name}")
            except (UnicodeDecodeError, json.JSONDecodeError):
                try:
                    # If that fails, try to read as binary and extract what we can
                    with open(metadata_file, 'rb') as f:
                        binary_data = f.read()
                    
                    # For crashpad binary metadata, try to extract useful information
                    metadata = {
                        'metadata_type': 'binary',
                        'metadata_size': len(binary_data),
                        'note': 'Binary crashpad metadata file'
                    }
                    
                    # Try to extract GUID from the binary data
                    # Look for GUID pattern (8-4-4-4-12 hex digits with dashes)
                    import re
                    ascii_data = binary_data.decode('ascii', errors='ignore')
                    guid_pattern = r'[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}'
                    guid_match = re.search(guid_pattern, ascii_data)
                    if guid_match:
                        metadata['crash_guid'] = guid_match.group(0)
                        self.logger.debug(f"Extracted crash GUID: {metadata['crash_guid']}")
                    
                    # Try to extract timestamps (if present as binary data)
                    if len(binary_data) >= 16:
                        try:
                            # Crashpad often stores timestamps as 64-bit values
                            import struct
                            # Try to extract potential timestamp values
                            for offset in [8, 16]:
                                if offset + 8 <= len(binary_data):
                                    timestamp_val = struct.unpack('<Q', binary_data[offset:offset+8])[0]
                                    # Check if it looks like a reasonable timestamp (after 2000, before 2100)
                                    if 946684800 < timestamp_val < 4102444800:  # Unix timestamps
                                        metadata[f'extracted_timestamp_{offset}'] = timestamp_val
                        except:
                            pass
                    
                    self.logger.debug(f"Loaded binary metadata for {minidump_path.name} ({len(binary_data)} bytes)")
                except Exception as e:
                    self.logger.warning(f"Failed to load metadata for {minidump_path.name}: {e}")
            except Exception as e:
                self.logger.warning(f"Failed to load metadata for {minidump_path.name}: {e}")
                
        # Upload using Sentry minidump endpoint
        self._upload_via_minidump_endpoint(minidump_path, crash_timestamp, metadata)
        
    def _upload_via_minidump_endpoint(self, minidump_path, crash_timestamp, metadata):
        """Upload minidump file to Sentry's minidump endpoint"""
        
        # Parse DSN to extract components
        from urllib.parse import urlparse, parse_qs
        parsed_dsn = urlparse(self.dsn)
        
        # Extract key from DSN (format: https://key@host/project_id)
        if '@' in parsed_dsn.netloc:
            key = parsed_dsn.username
            host = f"{parsed_dsn.hostname}:{parsed_dsn.port}" if parsed_dsn.port else parsed_dsn.hostname
        else:
            raise ValueError("Invalid DSN format - missing key")
            
        # Extract project ID from path
        project_id = parsed_dsn.path.strip('/')
        if not project_id:
            raise ValueError("Invalid DSN format - missing project ID")
            
        # Construct minidump endpoint URL
        minidump_url = f"{parsed_dsn.scheme}://{host}/api/{project_id}/minidump/?sentry_key={key}"
        
        # Prepare form data for multipart upload
        files = {}
        data = {}
        
        # Add the minidump file
        with open(minidump_path, 'rb') as f:
            files['upload_file_minidump'] = (minidump_path.name, f.read(), 'application/octet-stream')
        
        # Add metadata if available
        if metadata:
            if isinstance(metadata, dict):
                for key, value in metadata.items():
                    data[key] = str(value)
            
        # Add timestamp information
        data['sentry_timestamp'] = crash_timestamp.isoformat()
        
        # Add crash GUID - prefer extracted GUID from metadata, fallback to filename
        if metadata and 'crash_guid' in metadata:
            data['guid'] = metadata['crash_guid']
        else:
            # Use filename as crash ID if no GUID available
            data['guid'] = minidump_path.stem
            
        self.logger.debug(f"Uploading to: {minidump_url}")
        self.logger.debug(f"Form data: {data}")
        
        # Upload to Sentry minidump endpoint
        response = requests.post(
            minidump_url,
            files=files,
            data=data,
            timeout=30
        )
        
        if response.status_code == 200:
            self.logger.debug(f"Successfully uploaded {minidump_path.name}")
        else:
            error_msg = f"Upload failed with status {response.status_code}: {response.text}"
            self.logger.error(error_msg)
            raise Exception(error_msg)
            
    def batch_upload_with_progress(self, reports_directory, max_workers=4, dry_run=False):
        """Upload with progress bar and parallel processing"""
        try:
            minidump_files = self.validate_directory(reports_directory)
            
            if not minidump_files:
                return
                
            if dry_run:
                self.logger.info(f"DRY RUN: Would upload {len(minidump_files)} files")
                for file_path in minidump_files:
                    self.logger.info(f"  - {file_path.name}")
                return
                
            successful_uploads = 0
            failed_uploads = 0
            
            self.logger.info(f"Starting upload of {len(minidump_files)} files with {max_workers} workers")
            
            with ThreadPoolExecutor(max_workers=max_workers) as executor:
                # Submit all upload tasks
                future_to_file = {
                    executor.submit(self.upload_single_minidump, file): file 
                    for file in minidump_files
                }
                
                # Process completed tasks with progress bar
                with tqdm(total=len(minidump_files), desc="Uploading crashpad reports", unit="file") as pbar:
                    for future in as_completed(future_to_file):
                        file_path = future_to_file[future]
                        try:
                            future.result()
                            successful_uploads += 1
                            self.logger.debug(f"✓ Uploaded: {file_path.name}")
                            pbar.set_postfix(success=successful_uploads, failed=failed_uploads)
                        except Exception as e:
                            failed_uploads += 1
                            self.logger.error(f"✗ Failed: {file_path.name} - {e}")
                            pbar.set_postfix(success=successful_uploads, failed=failed_uploads)
                        
                        pbar.update(1)
                        
            self.logger.info(f"Upload complete: {successful_uploads} successful, {failed_uploads} failed")
            
            if failed_uploads > 0:
                self.logger.warning(f"Check crashpad_upload.log for details on failed uploads")
                
        except Exception as e:
            self.logger.error(f"Batch upload failed: {e}")
            raise

def parse_arguments():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(
        description="Upload crashpad reports to Sentry while preserving timestamps",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s --dsn "https://key@sentry.io/project" --directory "/path/to/reports"
  %(prog)s --dsn "https://key@sentry.io/project" --directory "/path/to/reports" --workers 8
  %(prog)s --dsn "https://key@sentry.io/project" --directory "/path/to/reports" --dry-run
        """
    )
    
    parser.add_argument(
        '--dsn',
        required=True,
        help='Sentry DSN (Data Source Name)'
    )
    
    parser.add_argument(
        '--directory', '-d',
        required=True,
        help='Directory containing crashpad reports (.dmp files)'
    )
    
    parser.add_argument(
        '--workers', '-w',
        type=int,
        default=4,
        help='Number of parallel upload workers (default: 4)'
    )
    
    parser.add_argument(
        '--dry-run',
        action='store_true',
        help='Show what would be uploaded without actually uploading'
    )
    
    parser.add_argument(
        '--verbose', '-v',
        action='store_true',
        help='Enable verbose logging'
    )
    
    parser.add_argument(
        '--org-slug',
        help='Sentry organization slug (for API uploads)'
    )
    
    parser.add_argument(
        '--project-slug',
        help='Sentry project slug (for API uploads)'
    )
    
    parser.add_argument(
        '--auth-token',
        help='Sentry auth token (for API uploads)'
    )
    
    return parser.parse_args()

def main():
    """Main entry point"""
    args = parse_arguments()
    
    # Validate DSN format
    if not args.dsn.startswith(('http://', 'https://')):
        print("Error: DSN must start with http:// or https://")
        return 1
        
    try:
        # Initialize uploader
        uploader = BatchCrashpadUploader(
            dsn=args.dsn,
            org_slug=args.org_slug,
            project_slug=args.project_slug,
            auth_token=args.auth_token
        )
        
        # Set log level
        if args.verbose:
            uploader.setup_logging(logging.DEBUG)
        else:
            uploader.setup_logging(logging.INFO)
            
        # Validate workers count
        if args.workers < 1 or args.workers > 20:
            uploader.logger.warning("Workers count should be between 1 and 20, using default: 4")
            args.workers = 4
            
        # Start upload
        uploader.logger.info(f"Crashpad Uploader starting...")
        uploader.logger.info(f"DSN: {args.dsn[:50]}...")
        uploader.logger.info(f"Reports directory: {args.directory}")
        uploader.logger.info(f"Workers: {args.workers}")
        
        if args.dry_run:
            uploader.logger.info("DRY RUN MODE - No files will be uploaded")
            
        uploader.batch_upload_with_progress(
            reports_directory=args.directory,
            max_workers=args.workers,
            dry_run=args.dry_run
        )
        
        return 0
        
    except KeyboardInterrupt:
        print("\nUpload interrupted by user")
        return 1
    except Exception as e:
        print(f"Error: {e}")
        return 1

if __name__ == "__main__":
    exit(main())