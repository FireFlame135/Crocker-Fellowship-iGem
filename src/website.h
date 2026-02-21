/* * ======================================================================================
 * Project:      AllSafe Food Scanner v1.0
 * File Path:    src/website.h
 * Description:  
 * This file serves as the frontend repository. It stores HTML, CSS, SVG Graphics, 
 * and JavaScript as constant strings in Flash memory.
 * * Key Features:
 * - Chunked Resources: CSS is split into Core and Button styles for efficient transmission.
 * - Vector Graphics: Contains encoded SVGs for both the favicon and the main logo.
 * - Interactive UI: Includes CSS for holographic animations and JS for AJAX requests.
 * ======================================================================================
 */

#ifndef WEBSITE_H
#define WEBSITE_H

// --- PART 1: HEAD & FAVICON ---
// Standard HTML headers and a URL-encoded SVG favicon (Shield icon).
const char HTML_HEAD[] = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="UTF-8">
    <title>AllSafe</title>
    <link rel="icon" href='data:image/svg+xml;utf8,<svg viewBox="0 0 75 92" xmlns="http://www.w3.org/2000/svg"><path fill="%2368b0ab" d="M20.9,56c4.3,6.2,9,11.9,15,16.5c2.2,1.7,5.4,1.3,6.3-1.7C46.2,56.5,54.2,41.5,64.8,30.9c3.4-3.4-1.9-8.7-5.3-5.3C48,37.1,39.4,53.2,35,68.8l6.3-1.7c-5.5-4.3-9.8-9.3-13.8-15c-2.7-3.9-9.2-0.2-6.5,3.8Z"/><polygon fill="%234b7c59" points="48 58.4 62.2 58.4 56.9 42.4 60.2 38.1 72.5 74.2 67.2 74.2 63 62.1 46.5 62.1 48 58.4"/><g><path fill="%2368b0ab" d="M37.4,0C31.8,0.2,28.2,4.9,24.1,8C16.5,15.6,1.2,13.1,1,19.6c-5.5,28.5,11.8,57.3,36.6,70.6l3.5-1.9c-1-0.6-1.1-2.5-1.2-2.5C22.1,76,8.3,58.8,5.4,38.4c-0.9-5.8-1-14.3,0.5-18.9c-0.2,0.5,0.4-0.3-0.1,0.1c-0.2,0.2-0.3,0.4-0.6,0.5c6.9,0.6,22.1-7.1,27.6-13.4c-1.7-1.3,3-1.8-4.2-1.9C25.4,4.6,25.4-0.4,28.6-0.2"/><path fill="%2368b0ab" d="M63.7,67.8c-6.6,9.3-15.5,16.8-25.4,22.5l-3.4-1.9c0.1,0.1,0.1-2,1.1-2.6C45.6,80.3,60.9,63.3,60.9,63.3h1.6"/><path fill="%2368b0ab" d="M66.5,52.4c4.2-10.4,5.8-23.7,3.3-32.7c0.2,0.5-0.4-0.3,0.1,0.1c0.2,0.2,0.3,0.4,0.6,0.5c-6.9,0.6-22.1-7.1-27.6-13.4c-1.7-1.3-3-1.8-4.2-1.9c-3.2-0.2-3.2-5.2,0-5c5.6,0.2,9.1,4.9,13.2,8c5,4.2,13,6.5,20.1,7.6C81.1,23,73.6,49.8,68.9,59.2"/></g></svg>'>
)=====";

// --- PART 2A: CORE CSS ---
// Defines layout, typography, menu boxes, and the modal overlay.
const char HTML_CSS_CORE[] = R"=====(
    <style>
        body { 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; 
            background: #FAF3DD; color: #333; margin: 0; min-height: 100vh; 
            display: flex; flex-direction: column; align-items: center; text-align: center; 
        }
        .container { width: 100%; max-width: 400px; padding: 20px; box-sizing: border-box; }

        /* MENU BOX */
        .menu-box { 
            background: #fff; border: 2px solid #8FC0A9; border-radius: 15px; 
            padding: 20px; text-align: left; box-shadow: 0 4px 10px rgba(74, 124, 89, 0.1);
        }
        .menu-title { 
            font-weight: bold; color: #4A7C59; border-bottom: 2px solid #C8D5B9; 
            padding-bottom: 10px; margin-bottom: 15px; font-size: 18px;
        }

        /* CHECKBOXES */
        .checkbox-item { 
            display: flex; align-items: center; padding: 12px 0; 
            cursor: pointer; font-size: 18px; color: #555;
            border-bottom: 1px solid #f0f0f0;
        }
        .checkbox-item:last-child { border-bottom: none; }
        
        input[type="checkbox"] { 
            width: 22px; height: 22px; margin-right: 15px; 
            accent-color: #4A7C59; cursor: pointer;
        }

        /* MODAL OVERLAY */
        #result-overlay { 
            display: none; position: fixed; top: 0; left: 0; width: 100%; height: 100%; z-index: 100; 
            flex-direction: column; justify-content: center; align-items: center; 
            background: rgba(0,0,0,0.6); backdrop-filter: blur(5px);
        }
        
        .modal-card {
            background: white; padding: 40px 30px; border-radius: 20px;
            width: 85%; max-width: 350px; text-align: center;
            box-shadow: 0 20px 50px rgba(0,0,0,0.3);
            animation: popIn 0.3s ease-out;
        }

        @keyframes popIn { from { transform: scale(0.8); opacity: 0; } to { transform: scale(1); opacity: 1; } }

        .status-icon { font-size: 60px; margin-bottom: 15px; display: block; }
        .alert-title { font-size: 26px; font-weight: 800; margin: 0 0 10px 0; line-height: 1.2;}
        .alert-sub { font-size: 18px; color: #666; margin-bottom: 30px; line-height: 1.4; }
        
        .safe-text { color: #4A7C59; }
        .unsafe-text { color: #D9534F; }
        .loading-text { color: #68B0AB; }

        .close-btn { 
            width: 100%; padding: 15px; border-radius: 10px; border: none; 
            font-size: 18px; font-weight: bold; cursor: pointer; 
            background: #e0e0e0; color: #333;
        }
)=====";

// --- PART 2B: BUTTON CSS (Holographic Effect) ---
// Separated to keep packet size small. Defines the hover, scale, and shimmer effects.
const char HTML_CSS_BTN[] = R"=====(
        /* BUTTONS */
        .btn { 
            display: block; width: 100%; border: none; cursor: pointer;
            position: relative; overflow: hidden; /* Essential for shimmer effect */
            background: #4A7C59; color: #fff; padding: 18px; 
            font-size: 20px; font-weight: bold; border-radius: 12px; 
            margin-top: 20px; 
            transition: all 0.5s ease;
            z-index: 1;
        }

        /* The Shimmer Gradient Layer */
        .btn::before {
            content: '';
            position: absolute;
            top: -50%;
            left: -50%;
            width: 200%;
            height: 200%;
            background: linear-gradient(
                0deg, 
                transparent, 
                transparent 30%, 
                rgba(0, 255, 255, 0.3) /* Cyan shimmer */
            );
            transform: rotate(-45deg);
            transition: all 0.5s ease;
            opacity: 0;
            z-index: -1;
        }

        /* Hover States */
        .btn:hover {
            transform: scale(1.05);
            box-shadow: 0 0 20px rgba(0, 255, 255, 0.5); /* Glowing cyan shadow */
        }
        
        .btn:hover::before {
            opacity: 1;
            transform: rotate(-45deg) translateY(100%);
        }

        .btn:disabled { 
            background: #8da595; 
            cursor: not-allowed; 
            transform: none; 
            box-shadow: none; 
        }
    </style>
</head>
)=====";

// --- PART 3: BODY START ---
const char HTML_BODY_TOP[] = R"=====(
<body>
    <div id="result-overlay">
        <div class="modal-card">
            <span id="modal-icon" class="status-icon"></span>
            <div id="modal-title" class="alert-title"></div>
            <div id="modal-msg" class="alert-sub"></div>
            <button class="close-btn" onclick="closeAlert()">CLOSE</button>
        </div>
    </div>
    
    <div class="container">
)=====";

// --- PART 4: FULL LOGO SVG ---
const char HTML_LOGO[] = R"=====(
    <svg id="Ebene_2" data-name="Ebene 2" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 167.51 90.43" style="width:180px; height:auto; margin: 20px 0;">
      <defs>
        <style>
          .cls-1 { letter-spacing: -.01em; } .cls-1, .cls-2, .cls-3 { font-size: 38px; }
          .cls-4 { fill: #68b0ab; } .cls-5 { font-family: MyriadPro-Regular, 'Myriad Pro'; }
          .cls-5, .cls-6 { fill: #4b7c59; } .cls-7 { font-size: 56.99px; } .cls-2 { letter-spacing: 0em; }
        </style>
      </defs>
      <g id="Ebene_1-2" data-name="Ebene 1">
        <g>
          <text class="cls-5" transform="translate(73.53 74.35)">
            <tspan class="cls-3" x="0" y="0">ll</tspan><tspan class="cls-7" x="17.93" y="0">S</tspan>
            <tspan class="cls-2" x="46.03" y="0">a</tspan><tspan class="cls-1" x="64.34" y="0">f</tspan>
            <tspan class="cls-3" x="74.95" y="0">e</tspan>
          </text>
          <path class="cls-4" d="M20.9,55.99c4.31,6.2,8.96,11.87,14.97,16.53,2.16,1.68,5.43,1.31,6.27-1.65,4.05-14.32,12.05-29.32,22.63-39.92,3.41-3.42-1.89-8.73-5.3-5.3-11.51,11.53-20.15,27.63-24.56,43.23l6.27-1.65c-5.49-4.26-9.84-9.32-13.79-15.01-2.74-3.94-9.24-.19-6.48,3.79h0Z"/>
          <polygon class="cls-6" points="48 58.41 62.2 58.42 56.9 42.42 60.15 38.12 72.45 74.2 67.15 74.2 63.04 62.12 46.48 62.13 48 58.41"/>
          <g>
            <path class="cls-4" d="M37.35,0c-5.58.24-9.14,4.86-13.24,8.03C16.54,15.56,1.22,13.05,1.02,19.63c-5.5,28.45,11.78,57.31,36.63,70.64.08.04.23.11.23.11l3.45-1.91s-.1-1.91-1.16-2.52c-17.77-9.81-31.6-27.02-34.46-47.39-.93-5.84-1.01-14.33.49-18.91-.18.49.36-.32-.06.08-.21.2-.26.35-.58.53,6.93.63,22.11-7.13,27.6-13.4-1.68-1.28,2.95-1.79,4.19-1.86-3.2-.17,3.22-5.17,0-5h0Z"/>
            <path class="cls-4" d="M63.68,67.82c-6.58,9.29-15.49,16.84-25.39,22.46-.1.06-.31.15-.31.15l-3.36-1.85s.04-1.99,1.14-2.62c9.59-5.48,24.86-22.52,24.86-22.52h1.56"/>
            <path class="cls-4" d="M66.47,52.37c4.17-10.35,5.8-23.69,3.27-32.71.18.49-.36-.32.06.08.21.2.26.35.58.53-6.94.63-22.11-7.13-27.6-13.4-1.68-1.28-2.95-1.79-4.19-1.86-3.2-.17-3.22-5.17,0-5,5.58.24,9.14,4.86,13.24,8.03,5.05,4.21,13,6.47,20.09,7.6,9.14,7.43,1.6,34.21-3.13,43.62"/></g></g></g>
    </svg>
)=====";

// --- PART 5: MENU ---
const char HTML_MENU[] = R"=====(
        <div class="menu-box">
            <div class="menu-title">Select Your Allergens</div>
            <label class="checkbox-item"><input type="checkbox" id="milk"> Milk</label>
            <label class="checkbox-item"><input type="checkbox" id="eggs"> Eggs</label>
            <label class="checkbox-item"><input type="checkbox" id="peanuts"> Peanuts</label>
            <label class="checkbox-item"><input type="checkbox" id="treenuts"> Tree Nuts</label>
            <label class="checkbox-item"><input type="checkbox" id="soy"> Soy</label>
            <label class="checkbox-item"><input type="checkbox" id="wheat"> Wheat</label>
            <label class="checkbox-item"><input type="checkbox" id="fish"> Fish</label>
            <label class="checkbox-item"><input type="checkbox" id="shellfish"> Shellfish</label>
        </div>
)=====";

// --- PART 6: CONTROLS ---
const char HTML_CONTROLS[] = R"=====(
        <button id="scanBtn" class="btn" onclick="scanFood()">SCAN FOOD</button>
        <p style="font-size:12px; color:#8FC0A9; margin-top:20px;">AllSafe Food Scanner v1.0</p>
    </div> 
)=====";

// --- PART 7: SCRIPTS ---
const char HTML_SCRIPTS[] = R"=====(
    <script>
        window.onload = function() {
            // Restore Checkboxes
            var inputs = document.querySelectorAll('input[type="checkbox"]');
            inputs.forEach(function(item) {
                var storedState = localStorage.getItem(item.id);
                if (storedState === 'true') item.checked = true;
                item.onclick = function() { localStorage.setItem(item.id, item.checked); };
            });
        };

        // --- ASYNC SCAN FUNCTION ---
        function scanFood() {
            var btn = document.getElementById("scanBtn");
            btn.disabled = true;
            btn.innerText = "SCANNING...";

            // Send background request to Arduino
            fetch('/T')
                .then(response => response.text())
                .then(result => {
                    // Result will be "RED" or "GREEN"
                    showModal(result.trim() === "GREEN");
                    
                    // Reset Button
                    btn.disabled = false;
                    btn.innerText = "SCAN FOOD";
                })
                .catch(error => {
                    console.log(error);
                    alert("Connection failed. Try again.");
                    btn.disabled = false;
                    btn.innerText = "SCAN FOOD";
                });
        }

        function showModal(isSafe) {
            var overlay = document.getElementById('result-overlay');
            var icon = document.getElementById('modal-icon');
            var title = document.getElementById('modal-title');
            var msg = document.getElementById('modal-msg');
            
            if (isSafe) {
                icon.innerText = "✅";
                title.innerText = "LIKELY SAFE";
                title.className = "alert-title safe-text";
                msg.innerText = "No contaminants detected.";
            } else {
                icon.innerText = "⚠️";
                title.innerText = "WARNING: DO NOT EAT";
                title.className = "alert-title unsafe-text";
                
                var bad = [];
                document.querySelectorAll('input:checked').forEach(function(i) {
                     bad.push(i.parentElement.innerText.trim());
                });
                
                if (bad.length > 0) {
                    var culprit = bad[Math.floor(Math.random() * bad.length)];
                    msg.innerText = "This food could potentially have: " + culprit;
                } else {
                    msg.innerText = "This food is likely UNSAFE.";
                }
            }
            overlay.style.display = 'flex';
        }

        function closeAlert() {
             document.getElementById('result-overlay').style.display = 'none';
        }
    </script>
</body>
</html>
)=====";

#endif