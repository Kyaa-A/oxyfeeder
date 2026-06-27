# 📱 OxyFeeder Deployment Guide

**Purpose:** Instructions for building and deploying your Flutter app to Android devices.

**When to use this:** After Phase 3 is complete and you want to install the app on the dedicated phone at the fishpond, or share with others.

---

## 📋 Table of Contents

1. [Prerequisites](#prerequisites)
2. [Development Deployment (Testing)](#development-deployment-testing)
3. [Production Deployment (Release Build)](#production-deployment-release-build)
4. [Installing on Dedicated Device](#installing-on-dedicated-device)
5. [Troubleshooting](#troubleshooting)
6. [App Updates](#app-updates)

---

## ✅ Prerequisites

Before deploying, make sure you have:

- [ ] Flutter installed and working (`flutter doctor` shows no critical issues)
- [ ] Android device with USB debugging enabled OR Android SDK installed
- [ ] Your OxyFeeder app code ready
- [ ] USB cable (for initial installation)

---

## 🧪 Development Deployment (Testing)

**Use this when:** You're still developing and want to test on a real phone.

### Method 1: Run via USB (Fastest for Development)

**A. Enable USB Debugging on your phone** (one-time setup)
1. Go to **Settings → About Phone**
2. Tap **Build Number** 7 times rapidly
3. Go back to **Settings → Developer Options**
4. Turn on **USB Debugging**

**B. Connect and Run**

**On WSL/Linux:**
```bash
cd /home/asnari/Project/OxyFeeder/app
$HOME/flutter/bin/flutter run
```

**On Windows:**
```powershell
cd "C:\Users\Asnari Pacalna\Project\OxyFeeder\app"
flutter run
```

**C. What happens:**
- Flutter builds the app in debug mode
- Installs it on your connected phone
- Shows live logs in the terminal
- Enables **hot reload** (press `r` to instantly update UI changes)

**Features:**
- ✅ Fastest iteration cycle
- ✅ Hot reload support
- ✅ Debug logging enabled
- ✅ DevTools available
- ❌ Larger app size (~50MB)
- ❌ Slower performance than release build

---

### Method 2: Wireless Debugging (No Cable Needed)

**A. Enable Wireless Debugging** (Android 11+)
1. Settings → Developer Options → **Wireless Debugging** → ON
2. Tap "Wireless Debugging"
3. Note the IP address and port (e.g., `192.168.1.100:12345`)

**B. Connect via ADB**
```bash
adb pair <ip>:<port>  # Enter pairing code when prompted
adb connect <ip>:<port>
```

**C. Run Flutter**
```bash
flutter run
```

Now you can develop without a USB cable!

---

## 🚀 Production Deployment (Release Build)

**Use this when:** You want the final, optimized app for installation at the fishpond or for sharing.

### Step 1: Build Release APK

**What is an APK?**
- Android Package file (.apk)
- The installable file for Android apps
- Like a .exe on Windows or .dmg on Mac

**Build command:**

**On WSL/Linux:**
```bash
cd /home/asnari/Project/OxyFeeder/app
$HOME/flutter/bin/flutter build apk --release
```

**On Windows:**
```powershell
cd "C:\Users\Asnari Pacalna\Project\OxyFeeder\app"
flutter build apk --release
```

**This will:**
- Compile your Dart code to native ARM machine code
- Optimize the app for performance
- Remove debug symbols and logging
- Compress resources
- Create a ~20-30MB APK file

**Build time:** 2-5 minutes (first build), 30-60 seconds (subsequent builds)

**Output location:**
```
app/build/app/outputs/flutter-apk/app-release.apk
```

---

### Step 2: Locate the APK

**On WSL:**
```bash
# The APK is here:
/home/asnari/Project/OxyFeeder/app/build/app/outputs/flutter-apk/app-release.apk

# Copy to Windows Downloads for easy access:
cp /home/asnari/Project/OxyFeeder/app/build/app/outputs/flutter-apk/app-release.apk /mnt/c/Users/"Asnari Pacalna"/Downloads/OxyFeeder.apk
```

**On Windows:**
```
C:\Users\Asnari Pacalna\Project\OxyFeeder\app\build\app\outputs\flutter-apk\app-release.apk
```

**Tip:** Rename it to something memorable like `OxyFeeder-v1.0.apk`

---

### Step 3: Install the APK

You have **3 options**:

#### Option A: Install via USB (Recommended for first install)

**1. Connect phone via USB**
**2. Run this command:**

```bash
adb install app/build/app/outputs/flutter-apk/app-release.apk
```

**3. Done!** The app appears in your app drawer.

---

#### Option B: Install via File Transfer

**1. Copy APK to phone**
- Connect phone via USB
- Drag `app-release.apk` to your phone's Downloads folder

**2. On your phone:**
- Open **Files** or **My Files** app
- Navigate to **Downloads**
- Tap `app-release.apk`
- Tap **Install**
- If prompted about "Install from unknown sources", tap **Settings** and enable it
- Tap **Install** again

**3. Done!**

---

#### Option C: Share via Cloud/Link

**For the dedicated fishpond phone or sharing with others:**

**1. Upload APK to cloud storage**
- Google Drive, Dropbox, OneDrive, etc.
- Generate a shareable link

**2. On the receiving phone:**
- Open the link in Chrome/browser
- Download the APK
- Tap to install (enable "Unknown sources" if needed)

**Security note:** Only share the APK with trusted people. Anyone with the file can install it.

---

## 📲 Installing on Dedicated Device (Fishpond Phone)

**Scenario:** You have a phone that stays at the fishpond permanently.

### Recommended Setup:

**1. Install the app** (use Option B or C above)

**2. Configure the phone:**

**A. Power Settings**
- Settings → Display → **Screen timeout: 30 minutes** (or Never, if option available)
- Settings → Battery → **Optimize battery usage** → Find OxyFeeder → Set to **Don't optimize**

**B. Auto-start on boot (if available)**
- Some launchers support this
- Or use Tasker/Automate app to auto-launch OxyFeeder on boot

**C. Prevent app killing**
- Settings → Apps → OxyFeeder → Battery → **Unrestricted**
- Settings → Apps → OxyFeeder → Permissions → Enable **Location** and **Bluetooth**

**D. Keep screen on (optional)**
- Settings → Developer Options → **Stay awake** → ON
- Only if phone is plugged into power permanently

**3. Mount the phone near the pond**
- Waterproof case recommended
- USB power adapter or power bank
- Within Bluetooth range of OxyFeeder hardware (~10 meters)

---

## 🐛 Troubleshooting

### Problem 1: "Build failed" during `flutter build apk`

**Check:**

**A. Run flutter doctor**
```bash
flutter doctor
```
Fix any issues it reports (especially Android SDK)

**B. Clean build**
```bash
flutter clean
flutter pub get
flutter build apk --release
```

**C. Check for code errors**
```bash
flutter analyze
```
Fix any errors reported

---

### Problem 2: App crashes immediately after install

**Possible causes:**

**A. Built with wrong Flutter version**
- Rebuild with the same Flutter you used for development
- Check: `flutter --version`

**B. Missing permissions**
- Android requires Bluetooth and Location permissions declared in AndroidManifest.xml
- Check file: `app/android/app/src/main/AndroidManifest.xml`
- Should include:
```xml
<uses-permission android:name="android.permission.BLUETOOTH"/>
<uses-permission android:name="android.permission.BLUETOOTH_ADMIN"/>
<uses-permission android:name="android.permission.BLUETOOTH_SCAN"/>
<uses-permission android:name="android.permission.BLUETOOTH_CONNECT"/>
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION"/>
```

**C. Check logs**
```bash
adb logcat | grep flutter
```
Look for error messages

---

### Problem 3: "Install blocked" on phone

**Solution:**

**A. Enable Unknown Sources**
- Settings → Security → **Install unknown apps**
- Find your browser/file manager → Enable
- OR Settings → Security → **Unknown sources** → ON

**B. Google Play Protect blocking**
- When install dialog appears, tap **More details**
- Tap **Install anyway**

---

### Problem 4: App size is too large

**Current size:** ~20-30MB is normal for Flutter apps

**To reduce size:**

**A. Build split APKs (by CPU architecture)**
```bash
flutter build apk --split-per-abi
```

This creates 3 APKs (~8-10MB each):
- `app-armeabi-v7a-release.apk` (32-bit ARM, most common)
- `app-arm64-v8a-release.apk` (64-bit ARM, newer phones)
- `app-x86_64-release.apk` (x86, rare)

**Use the correct one for your phone.** Most modern phones use arm64-v8a.

**B. Enable code shrinking** (already enabled in release builds)

---

### Problem 5: Bluetooth permissions not working

**On Android 12+ (API 31+):**

Bluetooth requires runtime permissions. The app should request these automatically on first launch.

**If not working:**

**A. Manually grant permissions**
- Settings → Apps → OxyFeeder → Permissions
- Enable: **Location** (Nearby devices), **Bluetooth**

**B. Check Android version compatibility**
- OxyFeeder requires Android 5.0 (API 21) minimum
- Bluetooth LE works best on Android 8.0+ (API 26+)

---

## 🔄 App Updates

**When you make changes to the app and want to update the installed version:**

### Development Updates (USB connected)

**Easiest way:**
```bash
flutter run
```
Flutter automatically uninstalls old version and installs new one.

---

### Production Updates

**A. Increment version number** (important!)

Edit `app/pubspec.yaml`:
```yaml
version: 1.0.0+1  # Change to 1.0.1+2
#        ↑ ↑              ↑     ↑
#     major.minor.patch+build
```

- Bump **patch** for bug fixes (1.0.1)
- Bump **minor** for new features (1.1.0)
- Bump **major** for breaking changes (2.0.0)
- Always increment **build number** (the number after +)

**B. Rebuild APK**
```bash
flutter build apk --release
```

**C. Uninstall old version first** (on phone):
- Settings → Apps → OxyFeeder → **Uninstall**
- OR via adb: `adb uninstall com.example.oxyfeeder_app`

**D. Install new version** (same methods as initial install)

**Note:** Uninstalling will delete all app data (settings, schedules). In the future, you can implement data persistence to survive updates.

---

## 📊 Build Types Comparison

| **Feature** | **Debug** (`flutter run`) | **Release** (`flutter build apk`) |
|-------------|--------------------------|-----------------------------------|
| **Size** | ~50MB | ~20-30MB |
| **Performance** | Slower (JIT compilation) | Fast (AOT compilation) |
| **Hot Reload** | ✅ Yes | ❌ No |
| **Debug Logs** | ✅ Enabled | ❌ Stripped |
| **DevTools** | ✅ Available | ❌ Not available |
| **Build Time** | 30-60 seconds | 2-5 minutes |
| **Use Case** | Active development | Final deployment |

**Recommendation:** Use **Debug** during development, **Release** for deployment.

---

## 🎯 Deployment Checklist

**Before deploying to production (fishpond), verify:**

### Code Readiness
- [ ] App tested thoroughly on your development phone
- [ ] All features working (Dashboard, Sensors, Settings)
- [ ] Bluetooth connection to hardware tested successfully
- [ ] No console errors or warnings
- [ ] Version number updated in pubspec.yaml

### Build Process
- [ ] `flutter analyze` shows no errors
- [ ] `flutter build apk --release` completes successfully
- [ ] APK file generated at correct location
- [ ] APK file renamed to meaningful name (e.g., OxyFeeder-v1.0.apk)

### Installation
- [ ] APK installs successfully on test phone
- [ ] App launches without crashes
- [ ] All permissions granted (Bluetooth, Location)
- [ ] Can connect to OxyFeeder hardware
- [ ] Dashboard shows live data from hardware

### Dedicated Device Setup
- [ ] App installed on fishpond phone
- [ ] Bluetooth paired with OxyFeeder hardware
- [ ] Phone power settings configured (screen timeout, battery optimization)
- [ ] Phone mounted securely near pond
- [ ] Within Bluetooth range of hardware (~10 meters)
- [ ] Phone has power source (charger/power bank)

**✅ If all checked:** Ready for deployment!

---

## 🚀 Future: Google Play Store (Optional)

**If you want to publish on Play Store later:**

### Requirements:
- Google Play Developer account ($25 one-time fee)
- App Bundle instead of APK
- App signing key
- Privacy policy
- Screenshots and descriptions

### Build command:
```bash
flutter build appbundle --release
```

Output: `app/build/app/outputs/bundle/release/app-release.aab`

**Note:** This is optional. For personal use at the fishpond, APK installation is sufficient.

---

## 💡 Quick Reference

### Most Common Commands

```bash
# Development (with hot reload)
flutter run

# Build release APK
flutter build apk --release

# Build split APKs (smaller size)
flutter build apk --split-per-abi

# Install APK to connected phone
adb install path/to/app-release.apk

# Uninstall app from phone
adb uninstall com.example.oxyfeeder_app

# Check connected devices
adb devices
flutter devices

# Clean build (when things go wrong)
flutter clean
flutter pub get

# Check for issues
flutter doctor
flutter analyze
```

### File Locations

**APK output:**
- WSL: `/home/asnari/Project/OxyFeeder/app/build/app/outputs/flutter-apk/app-release.apk`
- Windows: `C:\Users\Asnari Pacalna\Project\OxyFeeder\app\build\app\outputs\flutter-apk\app-release.apk`

**Version file:**
- `app/pubspec.yaml` (line ~19: `version: 1.0.0+1`)

**Android Manifest:**
- `app/android/app/src/main/AndroidManifest.xml`

---

## ✅ Success Criteria

**You've successfully deployed when:**

1. ✅ Release APK builds without errors
2. ✅ APK file is ~20-30MB in size
3. ✅ App installs on phone successfully
4. ✅ App launches without crashes
5. ✅ All permissions granted automatically or manually
6. ✅ Can connect to OxyFeeder hardware via Bluetooth
7. ✅ Dashboard shows live data from sensors
8. ✅ App runs stably for extended period (hours/days)

**When all criteria met:** 🎊 **Your OxyFeeder system is fully deployed!**

---

*Last Updated: 2025-11-22*
*For: OxyFeeder Project - Production Deployment*
