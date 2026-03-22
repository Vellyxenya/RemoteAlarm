import 'dart:io';
import 'package:flutter/material.dart';
import 'package:audioplayers/audioplayers.dart';
import '../services/audio_service.dart';
import '../services/storage_service.dart';
import '../utils/audio_utils.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  final AudioService _audioService = AudioService();
  final StorageService _storageService = StorageService();
  final AudioPlayer _audioPlayer = AudioPlayer();
  
  bool _isRecording = false;
  bool _isUploading = false;
  bool _isPlaying = false;
  String? _recordedFilePath;
  String _status = 'Ready to record';
  
  // Alarm selection
  String _selectedAlarm = 'None';
  final Map<String, String?> _alarmOptions = {
    'None': null,
    'Ding (Soft)': 'assets/sounds/alarm1.wav',
    'Chime (Major Chord)': 'assets/sounds/alarm2.wav',
    'Alert (Two-Tone)': 'assets/sounds/alarm3.wav',
  };

  @override
  void initState() {
    super.initState();
    _initializeServices();
    
    _audioPlayer.onPlayerStateChanged.listen((state) {
      if (mounted) {
        setState(() {
          _isPlaying = state == PlayerState.playing;
        });
      }
    });

    _audioPlayer.onPlayerComplete.listen((event) {
      if (mounted) {
        setState(() {
          _isPlaying = false;
          _status = 'Playback complete';
        });
      }
    });
  }

  Future<void> _initializeServices() async {
    await _audioService.initialize();
    await _storageService.initialize();
  }

  Future<void> _startRecording() async {
    if (_isPlaying) {
      await _audioPlayer.stop();
    }
    
    try {
      setState(() {
        _status = 'Recording...';
        _isRecording = true;
      });
      
      await _audioService.startRecording();
    } catch (e) {
      setState(() {
        _status = 'Error: $e';
        _isRecording = false;
      });
    }
  }

  Future<void> _stopRecording() async {
    try {
      final filePath = await _audioService.stopRecording();
      setState(() {
        _isRecording = false;
        _recordedFilePath = filePath;
        _status = 'Recording saved. Ready to upload.';
      });
    } catch (e) {
      setState(() {
        _status = 'Error: $e';
        _isRecording = false;
      });
    }
  }

  Future<void> _playRecording() async {
    if (_recordedFilePath != null) {
      try {
        await _audioPlayer.play(DeviceFileSource(_recordedFilePath!));
        setState(() {
          _status = 'Playing recording...';
        });
      } catch (e) {
        setState(() {
          _status = 'Playback error: $e';
        });
      }
    }
  }

  Future<void> _stopPlayback() async {
    await _audioPlayer.stop();
    setState(() {
      _status = 'Playback stopped';
    });
  }

  Future<void> _uploadAudio() async {
    if (_isPlaying) {
      await _audioPlayer.stop();
    }
    
    if (_recordedFilePath == null) {
      setState(() => _status = 'No recording to upload');
      return;
    }

    try {
      setState(() {
        _status = 'Preparing...';
        _isUploading = true;
      });
      
      String fileToUpload = _recordedFilePath!;
      final alarmAsset = _alarmOptions[_selectedAlarm];
      
      if (alarmAsset != null) {
        try {
          setState(() => _status = 'Merging alarm sound...');
          final alarmFile = await AudioUtils.extractAssetToTemp(alarmAsset);
          fileToUpload = await AudioUtils.mergeWavFiles(alarmFile.path, _recordedFilePath!);
        } catch (e) {
          debugPrint('Error merging audio: $e');
          if (mounted) {
            ScaffoldMessenger.of(context).showSnackBar(
              SnackBar(content: Text('Failed to add alarm: $e. Uploading recording only.')),
            );
          }
        }
      }

      setState(() {
        _status = 'Uploading...';
      });

      await _storageService.uploadAudio(fileToUpload);
      
      setState(() {
        _status = 'Upload successful!';
        _isUploading = false;
        _recordedFilePath = null;
      });

      // Show success message
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(
            content: Text('Audio uploaded successfully!'),
            backgroundColor: Colors.green,
          ),
        );
      }
    } catch (e) {
      setState(() {
        _status = 'Upload error: $e';
        _isUploading = false;
      });
    }
  }

  @override
  void dispose() {
    _audioService.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
        title: const Text('RemoteAlarm'),
      ),
      body: Center(
        child: Padding(
          padding: const EdgeInsets.all(24.0),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Icon(
                _isRecording ? Icons.mic : Icons.mic_none,
                size: 100,
                color: _isRecording ? Colors.red : Colors.grey,
              ),
              const SizedBox(height: 40),
              Text(
                _status,
                style: Theme.of(context).textTheme.titleLarge,
                textAlign: TextAlign.center,
              ),
              const SizedBox(height: 20),
              
              // Alarm Selection
              DropdownButton<String>(
                value: _selectedAlarm,
                icon: const Icon(Icons.arrow_downward),
                elevation: 16,
                style: const TextStyle(color: Colors.deepPurple),
                underline: Container(
                  height: 2,
                  color: Colors.deepPurpleAccent,
                ),
                onChanged: (_isRecording || _isUploading) ? null : (String? value) {
                  setState(() {
                    _selectedAlarm = value!;
                  });
                },
                items: _alarmOptions.keys.map<DropdownMenuItem<String>>((String value) {
                  return DropdownMenuItem<String>(
                    value: value,
                    child: Text(value),
                  );
                }).toList(),
              ),
              const SizedBox(height: 20),
              
              // Record button
              ElevatedButton.icon(
                onPressed: _isRecording || _isUploading
                    ? null
                    : _startRecording,
                icon: const Icon(Icons.fiber_manual_record),
                label: const Text('Start Recording'),
                style: ElevatedButton.styleFrom(
                  minimumSize: const Size(200, 50),
                  backgroundColor: Colors.red,
                  foregroundColor: Colors.white,
                ),
              ),
              const SizedBox(height: 16),
              
              // Stop button
              ElevatedButton.icon(
                onPressed: _isRecording ? _stopRecording : null,
                icon: const Icon(Icons.stop),
                label: const Text('Stop Recording'),
                style: ElevatedButton.styleFrom(
                  minimumSize: const Size(200, 50),
                ),
              ),
              const SizedBox(height: 16),

              // Play button
              ElevatedButton.icon(
                onPressed: (_recordedFilePath != null && !_isRecording && !_isUploading)
                    ? (_isPlaying ? _stopPlayback : _playRecording)
                    : null,
                icon: Icon(_isPlaying ? Icons.stop_circle_outlined : Icons.play_arrow),
                label: Text(_isPlaying ? 'Stop Playback' : 'Play Recording'),
                style: ElevatedButton.styleFrom(
                  minimumSize: const Size(200, 50),
                ),
              ),
              const SizedBox(height: 32),
              
              // Upload button
              ElevatedButton.icon(
                onPressed: (_recordedFilePath != null && !_isUploading)
                    ? _uploadAudio
                    : null,
                icon: _isUploading
                    ? const SizedBox(
                        width: 20,
                        height: 20,
                        child: CircularProgressIndicator(strokeWidth: 2),
                      )
                    : const Icon(Icons.upload),
                label: Text(_isUploading ? 'Uploading...' : 'Upload Audio'),
                style: ElevatedButton.styleFrom(
                  minimumSize: const Size(200, 50),
                  backgroundColor: Colors.green,
                  foregroundColor: Colors.white,
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}