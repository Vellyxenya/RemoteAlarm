import 'package:flutter/material.dart';
import '../services/audio_service.dart';
import '../services/storage_service.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  final AudioService _audioService = AudioService();
  final StorageService _storageService = StorageService();
  
  bool _isRecording = false;
  bool _isUploading = false;
  String? _recordedFilePath;
  String _status = 'Ready to record';

  @override
  void initState() {
    super.initState();
    _initializeServices();
  }

  Future<void> _initializeServices() async {
    await _audioService.initialize();
    await _storageService.initialize();
  }

  Future<void> _startRecording() async {
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

  Future<void> _uploadAudio() async {
    if (_recordedFilePath == null) {
      setState(() => _status = 'No recording to upload');
      return;
    }

    try {
      setState(() {
        _status = 'Uploading...';
        _isUploading = true;
      });

      await _storageService.uploadAudio(_recordedFilePath!);
      
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
              const SizedBox(height: 60),
              
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