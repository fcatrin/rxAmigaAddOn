package pandora.uae4all.sdl.vinput;

import android.content.Intent;
import android.util.Log;
import android.view.KeyEvent;

public class Mapper {
	private static final String LOGTAG = "vinput.Mapper"; 
	public static GenericJoystick genericJoystick = new GenericJoystick();
	
	public Mapper(Intent intent) {
		KeyTranslator.init();
		initVirtualEvents(intent);
		initGenericJoystick(intent);
	}
	
	private void initVirtualEvents(Intent intent) {
	    for(int i=0; i<GenericJoystick.eventNames.length; i++) {
	    	String keyName = GenericJoystick.eventNames[i];
	    	String keyNameLinux = intent.getStringExtra("kmap1" + keyName); // only 1 player in android touchscreen
	    	if (keyNameLinux!=null) {
	    		Log.d("REMAP", "Key for " + keyName + " is " + keyNameLinux);
	    		VirtualEvent event = KeyTranslator.translate(keyNameLinux);
	    		genericJoystick.virtualEvents[i] = event;
	    		Log.d("REMAP", "Linux key " + keyNameLinux + " mapped to event " + event);
	    	} else genericJoystick.virtualEvents[i] = null;
	    }
	}
	
	private void initGenericJoystick(Intent intent) {
		for(int i=0; i<GenericJoystick.eventNames.length; i++) {
			String eventName = GenericJoystick.eventNames[i];
			Integer keyCode = intent.getIntExtra("j1" + eventName, 0);
			if (keyCode>0) {
				Log.d(LOGTAG, "keyCode for " + eventName + ":" + keyCode);
				genericJoystick.originCode[i] = keyCode;
			}
		}
	}
	
	public VirtualEvent getVirtualEvent(int keyCode) {
		for(int i=0; i<genericJoystick.originCode.length; i++) {
			if (genericJoystick.originCode[i] == keyCode) {
				VirtualEvent ev = genericJoystick.virtualEvents[i];
				return ev;
			}
		}
		return null;
	}
}
