export function extractPeaks(audioBuffer, numPeaks) {
  const channel = audioBuffer.getChannelData(0);
  const samplesPerPeak = Math.floor(channel.length / numPeaks);
  const peaks = new Float32Array(numPeaks);

  for (let i = 0; i < numPeaks; i++) {
    let max = 0;
    const start = i * samplesPerPeak;
    const end = Math.min(start + samplesPerPeak, channel.length);
    for (let j = start; j < end; j++) {
      const abs = Math.abs(channel[j]);
      if (abs > max) max = abs;
    }
    peaks[i] = max;
  }
  return peaks;
}

export function drawWaveform(canvas, audioBuffer, color, pixelsPerSecond = 100) {
  const ctx = canvas.getContext('2d');
  const { width, height } = canvas;
  ctx.clearRect(0, 0, width, height);

  if (!audioBuffer) return;

  const numPeaks = width;
  const peaks = extractPeaks(audioBuffer, numPeaks);
  const midY = height / 2;

  // Gradient fill
  const gradient = ctx.createLinearGradient(0, 0, 0, height);
  gradient.addColorStop(0, color + '40');
  gradient.addColorStop(0.5, color + 'cc');
  gradient.addColorStop(1, color + '40');

  ctx.fillStyle = gradient;
  ctx.beginPath();
  ctx.moveTo(0, midY);

  // Top half
  for (let i = 0; i < numPeaks; i++) {
    const y = midY - peaks[i] * midY * 0.9;
    ctx.lineTo(i, y);
  }

  // Bottom half (mirror)
  for (let i = numPeaks - 1; i >= 0; i--) {
    const y = midY + peaks[i] * midY * 0.9;
    ctx.lineTo(i, y);
  }

  ctx.closePath();
  ctx.fill();

  // Center line
  ctx.strokeStyle = color + '60';
  ctx.lineWidth = 0.5;
  ctx.beginPath();
  ctx.moveTo(0, midY);
  ctx.lineTo(width, midY);
  ctx.stroke();
}
