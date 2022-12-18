package com.k2fsa.sherpa.ncnn

import android.Manifest
import android.content.pm.PackageManager
import android.media.AudioFormat
import android.media.AudioRecord
import android.media.MediaRecorder
import android.os.Bundle
import android.text.method.ScrollingMovementMethod
import android.util.Log
import android.widget.Button
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import kotlin.concurrent.thread

private const val TAG = "sherpa-ncnn"
private const val REQUEST_RECORD_AUDIO_PERMISSION = 200

class MainActivity : AppCompatActivity() {
    private val permissions: Array<String> = arrayOf(Manifest.permission.RECORD_AUDIO)

    // If there is a GPU and useGPU is true, we will use GPU
    // If there is no GPU and useGPU is true, we won't use GPU
    private val useGPU: Boolean = true

    private lateinit var model: SherpaNcnn
    private var audioRecord: AudioRecord? = null
    private lateinit var recordButton: Button
    private lateinit var textView: TextView
    private var recordingThread: Thread? = null

    private val audioSource = MediaRecorder.AudioSource.MIC
    private val sampleRateInHz = 16000
    private val channelConfig = AudioFormat.CHANNEL_IN_MONO

    // Note: We don't use AudioFormat.ENCODING_PCM_FLOAT
    // since the AudioRecord.read(float[]) needs API level >= 23
    // but we are targeting API level >= 21
    private val audioFormat = AudioFormat.ENCODING_PCM_16BIT

    @Volatile
    private var isRecording: Boolean = false

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        val permissionToRecordAccepted = if (requestCode == REQUEST_RECORD_AUDIO_PERMISSION) {
            grantResults[0] == PackageManager.PERMISSION_GRANTED
        } else {
            false
        }

        if (!permissionToRecordAccepted) {
            Log.e(TAG, "Audio record is disallowed")
            finish()
        }

        Log.i(TAG, "Audio record is permitted")
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        ActivityCompat.requestPermissions(this, permissions, REQUEST_RECORD_AUDIO_PERMISSION)

        Log.i(TAG, "Start to initialize model")
        initModel()
        Log.i(TAG, "Finished initializing model")

        recordButton = findViewById(R.id.record_button)
        recordButton.setOnClickListener { onclick() }

        textView = findViewById(R.id.my_text)
        textView.movementMethod = ScrollingMovementMethod()
    }

    private fun onclick() {
        if (!isRecording) {
            val ret = initMicrophone()
            if (!ret) {
                Log.e(TAG, "Failed to initialize microphone")
                return
            }
            Log.i(TAG, "state: ${audioRecord?.state}")
            audioRecord!!.startRecording()
            recordButton.setText(R.string.stop)
            isRecording = true
            model.reset()
            recordingThread = thread(true) {
                processSamples()
            }
            Log.i(TAG, "Started recording")
        } else {
            isRecording = false
            audioRecord!!.stop()
            audioRecord!!.release()
            audioRecord = null
            recordButton.setText(R.string.start)
            textView.text = model.text
            Log.i(TAG, "Stopped recording")
        }
    }

    private fun processSamples() {
        Log.i(TAG, "processing samples")

        val interval = 0.02 // i.e., 20 ms
        val bufferSize = (interval * sampleRateInHz).toInt() // in samples
        val buffer = ShortArray(bufferSize)

        while (isRecording) {
            val ret = audioRecord?.read(buffer, 0, buffer.size)
            if (ret != null && ret > 0) {
                val samples = FloatArray(ret) { buffer[it] / 32768.0f }
                model.decodeSamples(samples)
                runOnUiThread { textView.text = model.text }
            }
        }
    }

    private fun initMicrophone(): Boolean {
        if (ActivityCompat.checkSelfPermission(
                this,
                Manifest.permission.RECORD_AUDIO
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            ActivityCompat.requestPermissions(this, permissions, REQUEST_RECORD_AUDIO_PERMISSION)
            return false
        }

        val numBytes = AudioRecord.getMinBufferSize(sampleRateInHz, channelConfig, audioFormat)
        Log.i(
            TAG,
            "buffer size in milliseconds: ${numBytes * 1000.0f / sampleRateInHz}"
        )

        audioRecord = AudioRecord(
            audioSource,
            sampleRateInHz,
            channelConfig,
            audioFormat,
            numBytes * 2 // a sample has two bytes as we are using 16-bit PCM
        )
        return true
    }

    private fun initModel() {
        model = SherpaNcnn(
            assetManager = application.assets,
            modelConfig = getModelConfig(type = 1, useGPU = useGPU)!!,
            fbankConfig = getFbankConfig(),
        )
    }
}