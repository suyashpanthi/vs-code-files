import os
import uuid
import tempfile
import subprocess
from flask import Flask, request, jsonify, send_file
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

DOWNLOAD_DIR = os.path.join(tempfile.gettempdir(), 'daw_downloads')
os.makedirs(DOWNLOAD_DIR, exist_ok=True)


@app.route('/api/download', methods=['POST'])
def download_audio():
    data = request.get_json()
    if not data or 'url' not in data:
        return jsonify({'error': 'URL is required'}), 400

    url = data['url']
    filename = f'{uuid.uuid4().hex}.mp3'
    output_path = os.path.join(DOWNLOAD_DIR, filename)

    try:
        result = subprocess.run(
            [
                'yt-dlp',
                '--extract-audio',
                '--audio-format', 'mp3',
                '--audio-quality', '192K',
                '--no-playlist',
                '--output', output_path.replace('.mp3', '.%(ext)s'),
                url,
            ],
            capture_output=True,
            text=True,
            timeout=120,
        )

        if result.returncode != 0:
            return jsonify({'error': f'Download failed: {result.stderr[:200]}'}), 500

        # yt-dlp may produce the file with the right name
        if not os.path.exists(output_path):
            # Try to find the output file
            base = output_path.replace('.mp3', '')
            for ext in ['.mp3', '.m4a', '.webm', '.opus', '.wav']:
                candidate = base + ext
                if os.path.exists(candidate):
                    output_path = candidate
                    break
            else:
                return jsonify({'error': 'Output file not found'}), 500

        response = send_file(
            output_path,
            mimetype='audio/mpeg',
            as_attachment=True,
            download_name='audio.mp3',
        )

        # Clean up after sending
        @response.call_on_close
        def cleanup():
            try:
                os.remove(output_path)
            except OSError:
                pass

        return response

    except subprocess.TimeoutExpired:
        return jsonify({'error': 'Download timed out'}), 504
    except FileNotFoundError:
        return jsonify({'error': 'yt-dlp is not installed. Run: pip install yt-dlp'}), 500
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@app.route('/api/health', methods=['GET'])
def health():
    return jsonify({'status': 'ok'})


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
