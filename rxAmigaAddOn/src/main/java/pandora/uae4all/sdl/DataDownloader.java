// DO NOT EDIT THIS FILE - it is automatically generated, ALL YOUR CHANGES WILL BE OVERWRITTEN, edit the file under $JAVA_SRC_PATH dir
/*
Simple DirectMedia Layer
Java source code (C) 2009-2012 Sergii Pylypenko
  
This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:
  
1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required. 
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

package pandora.uae4all.sdl;


import android.widget.TextView;
import retrobox.v2.pandora.uae4all.sdl.R;
import java.util.zip.*;
import java.io.*;
import android.util.Log;
import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import android.content.res.Resources;
import java.lang.String;
import android.text.SpannedString;
import android.app.AlertDialog;
import android.content.DialogInterface;


class CountingInputStream extends BufferedInputStream
{

	private long bytesReadMark = 0;
	private long bytesRead = 0;

	public CountingInputStream(InputStream in, int size) {

		super(in, size);
	}

	public CountingInputStream(InputStream in) {

		super(in);
	}

	public long getBytesRead() {

		return bytesRead;
	}

	public synchronized int read() throws IOException {

		int read = super.read();
		if (read >= 0) {
			bytesRead++;
		}
		return read;
	}

	public synchronized int read(byte[] b, int off, int len) throws IOException {

		int read = super.read(b, off, len);
		if (read >= 0) {
			bytesRead += read;
		}
		return read;
	}

	public synchronized long skip(long n) throws IOException {

		long skipped = super.skip(n);
		if (skipped >= 0) {
			bytesRead += skipped;
		}
		return skipped;
	}

	public synchronized void mark(int readlimit) {

		super.mark(readlimit);
		bytesReadMark = bytesRead;
	}

	public synchronized void reset() throws IOException {

		super.reset();
		bytesRead = bytesReadMark;
	}
}


class DataDownloader extends Thread
{

	public static final String DOWNLOAD_FLAG_FILENAME = "libsdl-DownloadFinished-";

	class StatusWriter
	{
		private TextView Status;
		private MainActivity Parent;
		private SpannedString oldText = new SpannedString("");

		public StatusWriter( TextView _Status, MainActivity _Parent )
		{
			Status = _Status;
			Parent = _Parent;
		}
		public void setParent( TextView _Status, MainActivity _Parent )
		{
			synchronized(DataDownloader.this) {
				Status = _Status;
				Parent = _Parent;
				setText( oldText.toString() );
			}
		}
		
		public void setText(final String str)
		{
			class Callback implements Runnable
			{
				public TextView Status;
				public SpannedString text;
				public void run()
				{
					Status.setText(text);
				}
			}
			synchronized(DataDownloader.this) {
				Callback cb = new Callback();
				oldText = new SpannedString(str);
				cb.text = new SpannedString(str);
				cb.Status = Status;
				if( Parent != null && Status != null )
					Parent.runOnUiThread(cb);
			}
		}
		
	}
	public DataDownloader( MainActivity _Parent, TextView _Status )
	{
		Parent = _Parent;
		Status = new StatusWriter( _Status, _Parent );
		//Status.setText( "Connecting to " + Globals.DataDownloadUrl );
		outFilesDir = Globals.DataDir;
		DownloadComplete = false;
		this.start();
	}
	
	public void setStatusField(TextView _Status)
	{
		synchronized(this) {
			Status.setParent( _Status, Parent );
		}
	}

	@Override
	public void run()
	{
		/*
		Parent.keyListener = new BackKeyListener(Parent);
		String [] downloadFiles = Globals.DataDownloadUrl;
		int total = 0;
		int count = 0;
		for( int i = 0; i < downloadFiles.length; i++ )
		{
			if( downloadFiles[i].length() > 0 &&
				( Globals.OptionalDataDownload.length > i && Globals.OptionalDataDownload[i] ) ||
				( Globals.OptionalDataDownload.length <= i && downloadFiles[i].indexOf("!") == 0 ) )
				total += 1;
		}
		for( int i = 0; i < downloadFiles.length; i++ )
		{
			if( downloadFiles[i].length() > 0 &&
				( Globals.OptionalDataDownload.length > i && Globals.OptionalDataDownload[i] ) ||
				( Globals.OptionalDataDownload.length <= i && downloadFiles[i].indexOf("!") == 0 ) )
			{
				if( ! DownloadDataFile(downloadFiles[i].replace("<ARCH>", android.os.Build.CPU_ABI), DOWNLOAD_FLAG_FILENAME + String.valueOf(i) + ".flag", count+1, total, i) )
				{
					DownloadFailed = true;
					return;
				}
				count += 1;
			}
		}*/
		DownloadComplete = true;
		Parent.keyListener = null;
		initParent();
	}

	private void initParent()
	{
		class Callback implements Runnable
		{
			public MainActivity Parent;
			public void run()
			{
				Parent.initSDL();
			}
		}
		Callback cb = new Callback();
		synchronized(this) {
			cb.Parent = Parent;
			if(Parent != null)
				Parent.runOnUiThread(cb);
		}
	}
	
	private String getOutFilePath(final String filename)
	{
		return outFilesDir + "/" + filename;
	};

	public class BackKeyListener implements MainActivity.KeyEventsListener
	{
		MainActivity p;
		public BackKeyListener(MainActivity _p)
		{
			p = _p;
		}

		public void onKeyEvent(final int keyCode)
		{
			if( DownloadFailed )
				System.exit(1);

			AlertDialog.Builder builder = new AlertDialog.Builder(p);
			builder.setTitle(p.getResources().getString(R.string.cancel_download));
			builder.setMessage(p.getResources().getString(R.string.cancel_download) + (DownloadCanBeResumed ? " " + p.getResources().getString(R.string.cancel_download_resume) : ""));
			
			builder.setPositiveButton(p.getResources().getString(R.string.yes), new DialogInterface.OnClickListener()
			{
				public void onClick(DialogInterface dialog, int item) 
				{
					System.exit(1);
					dialog.dismiss();
				}
			});
			builder.setNegativeButton(p.getResources().getString(R.string.no), new DialogInterface.OnClickListener()
			{
				public void onClick(DialogInterface dialog, int item) 
				{
					dialog.dismiss();
				}
			});
			builder.setOnCancelListener(new DialogInterface.OnCancelListener()
			{
				public void onCancel(DialogInterface dialog)
				{
				}
			});
			AlertDialog alert = builder.create();
			alert.setOwnerActivity(p);
			alert.show();
		}
	}

	public StatusWriter Status;
	public boolean DownloadComplete = false;
	public boolean DownloadFailed = false;
	public boolean DownloadCanBeResumed = false;
	private MainActivity Parent;
	private String outFilesDir = null;
}
