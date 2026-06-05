# Video Preview Feature Plan

## 1. Settings & Configuration
Add new configurable options to the settings UI (`qimgv/gui/dialogs/settingsdialog.ui` and `settings.cpp/h`):
- **Enable Video Preview in Thumbnails**: A global flag to toggle video playback in thumbnails (folder view & slideshow thumbnail strip).
- **Preview Duration Limit**: A configurable time limit (in seconds) for the preview before it stops or loops (0 for infinite/full video length).
- **Folder View Preview Mode**: An option to choose between "Play all visible videos" and "Play only selected video".
- **Selected Video Sound**: A toggle to allow playing sound for the selected video preview (all other simultaneous video previews will be forced muted).

## 2. Thumbnail Playback Implementation
- Modify `ThumbnailWidget` to support hosting a scaled-down video player (`MpvWidget`) for video files.
- When `ThumbnailView` loads visible thumbnails, check if the item is a video and if previews are enabled.
- If enabled, initialize an `MpvWidget` for the thumbnail and start playback.
- **Sound Management**:
  - Non-selected video thumbnails will initialize with volume = 0 / muted = true.
  - The selected video thumbnail will initialize with the configured sound setting (muted or unmuted).
  - Listen for selection changes in `ThumbnailView` to update the mute state of videos dynamically if the mode allows sound for the selected video.
- **Duration Limit**: Use a `QTimer` or listen to `positionChanged` signals from the `MpvWidget` to detect when the preview duration has exceeded the configured limit, then loop back or stop depending on desired behavior.

## 3. Lifecycle & Performance Management
- Loading multiple `mpv` contexts could be heavy. We will only initialize `MpvWidget` instances for thumbnails that are currently visible on the screen (`loadVisibleThumbnails` in `ThumbnailView`).
- When a `ThumbnailWidget` scrolls out of view and is unloaded, we will cleanly destroy its `MpvWidget` to free up resources.
- For the "Play only selected video" mode, we will only instantiate the player for the single selected item, greatly improving performance.
