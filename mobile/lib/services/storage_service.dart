import 'dart:io';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:firebase_storage/firebase_storage.dart';
import 'package:uuid/uuid.dart';

class StorageService {
  final FirebaseAuth _auth = FirebaseAuth.instance;
  final FirebaseStorage _storage = FirebaseStorage.instance;
  final Uuid _uuid = const Uuid();

  Future<void> initialize() async {
    // Sign in anonymously if not already signed in
    if (_auth.currentUser == null) {
      await _auth.signInAnonymously();
    }
  }

  Future<String> uploadAudio(String filePath) async {
    // Ensure user is authenticated
    if (_auth.currentUser == null) {
      throw Exception('User not authenticated');
    }

    final file = File(filePath);
    if (!await file.exists()) {
      throw Exception('Audio file not found');
    }

    // Generate UUID for filename
    final fileName = '${_uuid.v4()}.wav';
    final remotePath = 'audio/$fileName';
    final storageRef = _storage.ref().child(remotePath);

    try {
      // Upload file
      await storageRef.putFile(
        file,
        SettableMetadata(
          contentType: 'audio/wav',
          customMetadata: {
            'uploadedAt': DateTime.now().toIso8601String(),
          },
        ),
      );

      // Delete local file after successful upload
      await file.delete();
      
      print('Audio uploaded successfully: $remotePath');
      return remotePath;
    } catch (e) {
      throw Exception('Upload failed: $e');
    }
  }

  String? get currentUserId => _auth.currentUser?.uid;
  bool get isAuthenticated => _auth.currentUser != null;
}