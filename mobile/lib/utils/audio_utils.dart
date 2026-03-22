import 'dart:io';
import 'dart:typed_data';
import 'package:flutter/services.dart' show rootBundle;
import 'package:path_provider/path_provider.dart';

class AudioUtils {
  /// Extracts an asset file to a temporary file.
  static Future<File> extractAssetToTemp(String assetPath) async {
    final byteData = await rootBundle.load(assetPath);
    final tempDir = await getTemporaryDirectory();
    final tempFile = File('${tempDir.path}/${assetPath.split('/').last}');
    await tempFile.writeAsBytes(byteData.buffer.asUint8List(), flush: true);
    return tempFile;
  }

  /// Merges two WAV files into a new file correctly handling headers.
  static Future<String> mergeWavFiles(String firstPath, String secondPath) async {
    final firstFile = File(firstPath);
    final secondFile = File(secondPath);

    if (!await firstFile.exists() || !await secondFile.exists()) {
      throw Exception('Input files do not exist.');
    }

    final firstBytes = await firstFile.readAsBytes();
    final secondBytes = await secondFile.readAsBytes();

    // 1. Robust Parse
    final firstWav = _parseWav(firstBytes);
    final secondWav = _parseWav(secondBytes);

    print('MERGE DEBUG:');
    print('Alarm: ${firstWav.sampleRate}Hz, Channels: ${firstWav.channels}');
    print('Voice: ${secondWav.sampleRate}Hz, Channels: ${secondWav.channels}');

    // 2. Determine Target Format
    // Use the voice recording's attributes as target to preserve its pitch/speed
    // But default to Mono output unless we really want Stereo
    final targetSampleRate = secondWav.sampleRate;
    final targetChannels = 1; // Output Mono for simplicity and consistency with current code requirements

    // 3. Process Audio Data (Resample & Downmix if needed)
    final firstData = _processAudioData(firstWav, targetSampleRate, targetChannels);
    final secondData = _processAudioData(secondWav, targetSampleRate, targetChannels);

    // 4. Create new WAV file
    final totalDataSize = firstData.length + secondData.length;
    final totalFileSize = 36 + totalDataSize;

    final outputHeader = Uint8List(44);
    final view = ByteData.view(outputHeader.buffer);

    // RIFF Chunk
    view.setUint32(0, 0x52494646, Endian.big); // "RIFF"
    view.setUint32(4, totalFileSize, Endian.little); // File Size - 8
    view.setUint32(8, 0x57415645, Endian.big); // "WAVE"

    // fmt Chunk
    view.setUint32(12, 0x666d7420, Endian.big); // "fmt "
    view.setUint32(16, 16, Endian.little); // Chunk size (16 for PCM)
    view.setUint16(20, 1, Endian.little); // AudioFormat (1 = PCM)
    view.setUint16(22, targetChannels, Endian.little); // NumChannels
    view.setUint32(24, targetSampleRate, Endian.little); // SampleRate
    view.setUint32(28, targetSampleRate * targetChannels * 2, Endian.little); // ByteRate
    view.setUint16(32, targetChannels * 2, Endian.little); // BlockAlign
    view.setUint16(34, 16, Endian.little); // BitsPerSample (16)

    // data Chunk
    view.setUint32(36, 0x64617461, Endian.big); // "data"
    view.setUint32(40, totalDataSize, Endian.little); // Data Size

    // Write file
    final tempDir = await getTemporaryDirectory();
    final timestamp = DateTime.now().millisecondsSinceEpoch;
    final outputPath = '${tempDir.path}/merged_$timestamp.wav';
    final outputFile = File(outputPath);
    final outputSink = outputFile.openWrite();
    
    outputSink.add(outputHeader);
    outputSink.add(firstData);
    outputSink.add(secondData);
    
    await outputSink.close();
    
    return outputPath;
  }

  static _WavInfo _parseWav(Uint8List bytes) {
    final view = ByteData.view(bytes.buffer);
    
    if (view.getUint32(0, Endian.big) != 0x52494646) throw Exception('Not a RIFF file'); // RIFF
    if (view.getUint32(8, Endian.big) != 0x57415645) throw Exception('Not a WAVE file'); // WAVE

    int? infoSampleRate;
    int? infoChannels;
    Uint8List? infoData;

    int offset = 12;
    while (offset < bytes.length) {
      if (offset + 8 > bytes.length) break;
      
      final chunkId = view.getUint32(offset, Endian.big);
      final chunkSize = view.getUint32(offset + 4, Endian.little);
      
      // "fmt " chunk
      if (chunkId == 0x666d7420) {
        infoChannels = view.getUint16(offset + 8 + 2, Endian.little);
        infoSampleRate = view.getUint32(offset + 8 + 4, Endian.little);
      }
      
      // "data" chunk
      if (chunkId == 0x64617461) {
        final start = offset + 8;
        // Limit actual end to file size in case of truncation
        final end = (start + chunkSize) > bytes.length ? bytes.length : (start + chunkSize);
        infoData = bytes.sublist(start, end);
        // Don't break immediately if we haven't found fmt yet
        if (infoSampleRate != null) break; 
      }
      
      // Handle padding byte for odd chunk sizes
      offset += 8 + chunkSize + (chunkSize % 2);
    }

    // Fallback scan for fmt if not found
    if (infoSampleRate == null) {
       print('WARNING: fmt chunk not found by standard parse, scanning...');
       for (int i = 12; i < bytes.length - 24; i++) {
        if (bytes[i] == 0x66 && bytes[i+1] == 0x6d && bytes[i+2] == 0x74 && bytes[i+3] == 0x20) {
             infoChannels = view.getUint16(i + 8 + 2, Endian.little);
             infoSampleRate = view.getUint32(i + 8 + 4, Endian.little);
             break;
        }
       }
    }

    // Fallback scan for data if not found
    if (infoData == null) {
       print('WARNING: data chunk not found by standard parse, scanning...');
       for (int i = 12; i < bytes.length - 8; i++) {
        if (bytes[i] == 0x64 && bytes[i+1] == 0x61 && bytes[i+2] == 0x74 && bytes[i+3] == 0x61) {
             final size = view.getUint32(i + 4, Endian.little);
             final end = (i + 8 + size) > bytes.length ? bytes.length : (i + 8 + size);
             infoData = bytes.sublist(i + 8, end);
             break;
        }
       }
    }

    if (infoData == null) throw Exception('No data chunk found');

    return _WavInfo(infoSampleRate ?? 44100, infoChannels ?? 1, infoData);
  }

  static Uint8List _processAudioData(_WavInfo wav, int targetRate, int targetChannels) {
    Int16List samples;
    
    // 1. Convert bytes to Int16 samples
    final buffer = wav.data.buffer;
    final offset = wav.data.offsetInBytes;
    final length = wav.data.lengthInBytes ~/ 2;
    // Check if aligned
    if (offset % 2 == 0) {
       samples = Int16List.view(buffer, offset, length);
    } else {
       // Copy to aligned buffer if necessary
       final aligned = Uint8List.fromList(wav.data);
       samples = Int16List.view(aligned.buffer);
    }

    // 2. Mixdown to Mono if needed
    Int16List monoSamples;
    if (wav.channels == 2 && targetChannels == 1) {
        monoSamples = Int16List(samples.length ~/ 2);
        for (int i = 0; i < monoSamples.length; i++) {
            // (L + R) / 2
            monoSamples[i] = ((samples[2*i] + samples[2*i+1]) ~/ 2);
        }
    } else {
        monoSamples = samples;
    }

    // 3. Resample using Linear Interpolation
    if (wav.sampleRate == targetRate) {
        return monoSamples.buffer.asUint8List(monoSamples.offsetInBytes, monoSamples.lengthInBytes);
    }

    final ratio = wav.sampleRate / targetRate;
    final targetLength = (monoSamples.length / ratio).floor();
    final resampled = Int16List(targetLength);

    for (int i = 0; i < targetLength; i++) {
        final position = i * ratio;
        final index = position.floor();
        final fraction = position - index;

        if (index + 1 < monoSamples.length) {
            final a = monoSamples[index];
            final b = monoSamples[index + 1];
            resampled[i] = (a + (b - a) * fraction).round();
        } else if (index < monoSamples.length) {
            resampled[i] = monoSamples[index];
        }
    }

    return resampled.buffer.asUint8List(resampled.offsetInBytes, resampled.lengthInBytes);
  }
}

class _WavInfo {
  final int sampleRate;
  final int channels;
  final Uint8List data;
  _WavInfo(this.sampleRate, this.channels, this.data);
}