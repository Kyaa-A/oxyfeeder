# Your Essential Flutter Command List

---

## 1. Project Management & Dependencies

These commands help you set up and manage your project and its packages.

| **Command** | **What It Does** | **When to Use It** |
|--------------|------------------|--------------------|
| `flutter create <project_name>` | Creates a brand new, clean Flutter project. | You've already done this, but it's the first command for any new project. |
| `flutter pub add <package_name>` | Finds the latest version of a package (e.g., for Bluetooth) and adds it to your **pubspec.yaml** file automatically. | Crucial. When you need to add new functionality, like `flutter pub add flutter_blue_plus`. |
| `flutter pub get` | Downloads all the packages listed in your **pubspec.yaml** file into your project. | After you manually edit **pubspec.yaml** or when you pull new code from a repository. |

---

## 2. Development & Running (Your Most-Used Commands)

These are the commands you'll use every day while coding.

| **Command** | **What It Does** | **When to Use It** |
|--------------|------------------|--------------------|
| `flutter run` | Builds and runs your app in **Debug Mode** on a connected device. Enables Hot Reload. | This is your main command for development. Pressing **F5** in Cursor does this for you. |
| `flutter devices` | Lists all connected devices (physical phones) and running emulators that Flutter can see. | Very important. Use this before running your app to see the available `device_id`s. |
| `flutter run -d <device_id>` | Runs your app on a specific device or emulator. The `device_id` comes from the `flutter devices` command. | When you have multiple devices/emulators running and want to target just one (e.g., `flutter run -d emulator-5554`). |

---

## 3. Building & Deployment (The “Long Game”)

These commands create the final, production-ready app files.

| **Command** | **What It Does** | **When to Use It** |
|--------------|------------------|--------------------|
| `flutter build apk` | Compiles an optimized, **Release Mode** `.apk` file. | Your primary build command for now. Use this to create the file you'll install on the dedicated phone near the fishpond or share with testers. |
| `flutter build appbundle` | Compiles an optimized, **Release Mode** `.aab` file. | For the future. Use this only when you're ready to publish your app on the Google Play Store. |
| `flutter clean` | Deletes the build directory and other temporary files. | When you have a strange build error that doesn’t make sense. It forces a completely fresh build. |

---

## 4. Troubleshooting & Maintenance (Your Lifesavers)

These commands help you fix problems with your Flutter installation and keep it up to date.

| **Command** | **What It Does** | **When to Use It** |
|--------------|------------------|--------------------|
| `flutter doctor` | Your #1 diagnostic tool. Checks your environment and reports any setup issues. | Run this first whenever something is wrong. If builds are failing or devices aren’t found, `flutter doctor` will tell you why. |
| `flutter doctor -v` | Shows a more detailed ("verbose") version of the doctor report. | When the basic `flutter doctor` doesn’t give enough information to solve a problem. |
| `flutter doctor --android-licenses` | Asks you to review and accept the licenses for the Android SDK tools. | A one-time setup step that `flutter doctor` will tell you to run if needed. |
| `flutter upgrade` | Upgrades your Flutter SDK to the latest available version. | Do this every few months to get the latest features, performance improvements, and bug fixes from the Flutter team. |

---

## Summary / Your Core Workflow

For your day-to-day work on the **OxyFeeder app**, you will mostly live by these three commands:

1. `flutter doctor` — *To check if your setup is healthy*  
2. `flutter run` — *To test and debug your app on an emulator or phone*  
3. `flutter build apk` — *To create the final version for installation*

---

✅ **Highlights:**
- Tables are perfectly aligned and readable in Markdown.  
- Ready to paste directly into **Cursor**, **Notion**, or any `.md` documentation.  
- Clean structure mirrors professional developer documentation.

Would you like me to include icons or color badges (like 🧠 ⚙️ 🚀) for quick visual scanning in your version?
