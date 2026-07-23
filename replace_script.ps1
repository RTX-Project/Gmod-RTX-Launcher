$lines = Get-Content rtx-launcher\main.cpp -Encoding UTF8
$start = -1
$end = -1
for ($i = 0; $i -lt $lines.Length; $i++) {
    if ($lines[$i] -match '^static void DoLaunchGame\(\) \{') {
        $start = $i
    }
    if ($start -ge 0 -and $lines[$i] -match '^static void DoStop\(\) \{') {
        $end = $i - 1
        while ($lines[$end].Trim() -eq '') {
            $end--
        }
        break
    }
}
if ($start -ge 0 -and $end -ge $start) {
    Write-Host "Replacing lines $start to $end"
    $newLines = $lines[0..($start-1)] + (Get-Content new_DoLaunchGame.cpp -Encoding UTF8) + "" + $lines[($end+1)..($lines.Length-1)]
    Set-Content rtx-launcher\main.cpp $newLines -Encoding UTF8
    Write-Host "Success"
} else {
    Write-Host "Could not find DoLaunchGame bounds"
}

// Trajectory ID: 3c754176-f0cd-4441-955c-8e7452236cf2
Error: HTTP 503 Service Unavailable
Sherlog: 
TraceID: 0x931a5cbbbf75027
Headers: {"Alt-Svc":["h3=\":443\"; ma=2592000,h3-29=\":443\"; ma=2592000"],"Content-Length":["415"],"Content-Type":["text/event-stream"],"Date":["Fri, 17 Jul 2026 16:26:49 GMT"],"Server":["ESF"],"Server-Timing":["gfet4t7; dur=447"],"Vary":["Origin","X-Origin","Referer"],"X-Cloudaicompanion-Trace-Id":["931a5cbbbf75027"],"X-Content-Type-Options":["nosniff"],"X-Frame-Options":["SAMEORIGIN"],"X-Xss-Protection":["0"]}

{
  "error": {
    "code": 503,
    "details": [
      {
        "@type": "type.googleapis.com/google.rpc.ErrorInfo",
        "domain": "cloudcode-pa.googleapis.com",
        "metadata": {
          "model": "claude-sonnet-4-6"
        },
        "reason": "MODEL_CAPACITY_EXHAUSTED"
      }
    ],
    "message": "No capacity available for model claude-sonnet-4-6 on the server",
    "status": "UNAVAILABLE"
  }
}