/* ==========================================================
   SMART ENVIRONMENT MONITOR - DASHBOARD MODULE (dashboard.js)
   Handles Auth Protection, Real-Time Firestore Snapshot Listener,
   Live Canvas Charting, Hardware Simulation, and Logout.
   ========================================================== */

import { auth, db } from "./firebase.js";
import {
  onAuthStateChanged,
  signOut
} from "https://www.gstatic.com/firebasejs/12.1.0/firebase-auth.js";

import {
  doc,
  onSnapshot,
  setDoc,
  getDoc,
  serverTimestamp
} from "https://www.gstatic.com/firebasejs/12.1.0/firebase-firestore.js";

// DOM Elements
const userEmailEl = document.getElementById("userEmail");
const userAvatarEl = document.getElementById("userAvatar");
const logoutButton = document.getElementById("logoutButton");
const lastUpdatedEl = document.getElementById("lastUpdated");
const cloudStatusDot = document.getElementById("cloudStatusDot");
const cloudStatusText = document.getElementById("cloudStatusText");

// Metric Elements
const tempEl = document.getElementById("temperature");
const humidityEl = document.getElementById("humidity");
const airQualityEl = document.getElementById("airQuality");
const smokeLevelEl = document.getElementById("smokeLevel");

const tempStatusEl = document.getElementById("tempStatus");
const humidityStatusEl = document.getElementById("humidityStatus");
const airStatusEl = document.getElementById("airStatus");
const smokeStatusEl = document.getElementById("smokeStatus");

// Simulator buttons
const simNormalBtn = document.getElementById("simNormalBtn");
const simWarningBtn = document.getElementById("simWarningBtn");

// Mobile nav
const mobileNavToggle = document.getElementById("mobileNavToggle");
const sidebarEl = document.getElementById("sidebar");

// Telemetry History Buffer for Canvas Chart
const MAX_HISTORY_POINTS = 12;
const telemetryHistory = [
  { time: "12:50", temp: 23.8, humidity: 52 },
  { time: "12:52", temp: 24.1, humidity: 53 },
  { time: "12:54", temp: 24.0, humidity: 54 },
  { time: "12:56", temp: 24.3, humidity: 55 },
  { time: "12:58", temp: 24.5, humidity: 55 }
];

// ------------------------------------------------------------------
// 1. Authentication Guard
// ------------------------------------------------------------------
onAuthStateChanged(auth, async (user) => {
  if (!user) {
    console.log("No authenticated user found. Redirecting to login.html");
    window.location.replace("login.html");
    return;
  }

  // Display user email
  if (userEmailEl) {
    userEmailEl.textContent = user.email || "Authenticated User";
  }

  // Display user avatar letter
  if (userAvatarEl && user.email) {
    userAvatarEl.textContent = user.email.charAt(0).toUpperCase();
  }

  // Attempt to fetch custom user display name from Firestore: users/{user.uid}
  try {
    const userDocRef = doc(db, "users", user.uid);
    const userDocSnap = await getDoc(userDocRef);
    if (userDocSnap.exists()) {
      const userData = userDocSnap.data();
      if (userData.name && userEmailEl) {
        userEmailEl.title = `Signed in as ${userData.name} (${user.email})`;
      }
    }
  } catch (profileErr) {
    console.warn("Could not fetch user profile document:", profileErr);
  }

  // Initialize Real-time Firestore Telemetry stream
  initFirestoreRealtimeStream();
});

// ------------------------------------------------------------------
// 2. Logout Handler
// ------------------------------------------------------------------
if (logoutButton) {
  logoutButton.addEventListener("click", async () => {
    try {
      logoutButton.disabled = true;
      logoutButton.textContent = "Logging out...";
      await signOut(auth);
      console.log("User successfully logged out");
      window.location.replace("login.html");
    } catch (err) {
      console.error("Logout failed:", err);
      alert("Failed to logout. Please check your network.");
      logoutButton.disabled = false;
      logoutButton.textContent = "Logout";
    }
  });
}

// ------------------------------------------------------------------
// 3. Real-Time Cloud Firestore Listener: sensorData/latest
// ------------------------------------------------------------------
let unsubscribeSnapshot = null;

function initFirestoreRealtimeStream() {
  if (unsubscribeSnapshot) {
    unsubscribeSnapshot();
  }

  const latestDocRef = doc(db, "sensorData", "latest");

  unsubscribeSnapshot = onSnapshot(
    latestDocRef,
    (snapshot) => {
      if (snapshot.exists()) {
        const data = snapshot.data();
        console.log("Real-time telemetry update received from Firestore:", data);
        updateDashboardUI(data);
      } else {
        console.log("No document found at sensorData/latest. Awaiting first telemetry push.");
        if (lastUpdatedEl) {
          lastUpdatedEl.textContent = "Awaiting initial ESP32 telemetry push...";
        }
      }

      // Update cloud indicator
      if (cloudStatusDot) cloudStatusDot.className = "pulse-dot online";
      if (cloudStatusText) cloudStatusText.textContent = "Firestore Live";
    },
    (error) => {
      console.error("Firestore snapshot listener error:", error);
      if (cloudStatusDot) cloudStatusDot.className = "pulse-dot";
      if (cloudStatusText) cloudStatusText.textContent = "Sync Paused";
      if (lastUpdatedEl) {
        lastUpdatedEl.textContent = "Listener error: " + error.message;
      }
    }
  );
}

// ------------------------------------------------------------------
// 4. Update Dashboard UI & Badges
// ------------------------------------------------------------------
function updateDashboardUI(data) {
  const now = new Date();
  const timeStr = now.toLocaleTimeString([], { hour: "2-digit", minute: "2-digit", second: "2-digit" });

  // 1. Temperature
  const tempVal = typeof data.temperature !== "undefined" ? parseFloat(data.temperature) : null;
  if (tempVal !== null && !isNaN(tempVal)) {
    tempEl.textContent = `${tempVal.toFixed(1)} °C`;
    if (tempVal > 35) {
      setBadge(tempStatusEl, "alert", "High Heat");
    } else if (tempVal > 28) {
      setBadge(tempStatusEl, "warning", "Elevated");
    } else if (tempVal < 16) {
      setBadge(tempStatusEl, "warning", "Low");
    } else {
      setBadge(tempStatusEl, "good", "Normal");
    }
  } else {
    tempEl.textContent = "-- °C";
  }

  // 2. Humidity
  const humVal = typeof data.humidity !== "undefined" ? parseFloat(data.humidity) : null;
  if (humVal !== null && !isNaN(humVal)) {
    humidityEl.textContent = `${humVal.toFixed(1)} %`;
    if (humVal > 75) {
      setBadge(humidityStatusEl, "warning", "High Moisture");
    } else if (humVal < 30) {
      setBadge(humidityStatusEl, "warning", "Dry Air");
    } else {
      setBadge(humidityStatusEl, "good", "Optimal");
    }
  } else {
    humidityEl.textContent = "-- %";
  }

  // 3. Air Quality
  if (typeof data.airQuality !== "undefined") {
    const aqStr = String(data.airQuality);
    airQualityEl.textContent = aqStr;
    const lowerAq = aqStr.toLowerCase();
    if (lowerAq.includes("poor") || lowerAq.includes("hazardous") || lowerAq.includes("bad")) {
      setBadge(airStatusEl, "alert", "Poor Quality");
    } else if (lowerAq.includes("moderate") || lowerAq.includes("fair")) {
      setBadge(airStatusEl, "warning", "Moderate");
    } else {
      setBadge(airStatusEl, "good", "Good (Clean)");
    }
  } else {
    airQualityEl.textContent = "--";
  }

  // 4. Smoke Level
  if (typeof data.smokeLevel !== "undefined") {
    const smokeStr = String(data.smokeLevel);
    smokeLevelEl.textContent = smokeStr;
    const lowerSmoke = smokeStr.toLowerCase();
    if (lowerSmoke.includes("alert") || lowerSmoke.includes("detect") || lowerSmoke.includes("high") || lowerSmoke.includes("danger")) {
      setBadge(smokeStatusEl, "alert", "Smoke Alert!");
    } else if (lowerSmoke.includes("warning")) {
      setBadge(smokeStatusEl, "warning", "Trace Detected");
    } else {
      setBadge(smokeStatusEl, "good", "Safe");
    }
  } else {
    smokeLevelEl.textContent = "--";
  }

  // Timestamp
  if (lastUpdatedEl) {
    lastUpdatedEl.textContent = `Last synchronized: ${timeStr}`;
  }

  // Append data point to history buffer for Canvas Chart
  if (tempVal !== null && humVal !== null) {
    const chartTime = now.toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" });
    telemetryHistory.push({ time: chartTime, temp: tempVal, humidity: humVal });
    if (telemetryHistory.length > MAX_HISTORY_POINTS) {
      telemetryHistory.shift();
    }
    renderTelemetryChart();
  }
}

// Helper: Set status badge class and label
function setBadge(el, statusType, text) {
  if (!el) return;
  el.className = `status-pill ${statusType}`;
  el.textContent = text;
}

// ------------------------------------------------------------------
// 5. Native Canvas Telemetry Chart Renderer
// ------------------------------------------------------------------
const canvas = document.getElementById("telemetryChart");
const ctx = canvas ? canvas.getContext("2d") : null;

function renderTelemetryChart() {
  if (!canvas || !ctx) return;

  // Handle responsive DPR scaling
  const dpr = window.devicePixelRatio || 1;
  const rect = canvas.getBoundingClientRect();
  canvas.width = rect.width * dpr;
  canvas.height = rect.height * dpr;
  ctx.resetTransform();
  ctx.scale(dpr, dpr);

  const w = rect.width;
  const h = rect.height;
  const padding = { top: 20, right: 30, bottom: 35, left: 45 };
  const chartW = w - padding.left - padding.right;
  const chartH = h - padding.top - padding.bottom;

  ctx.clearRect(0, 0, w, h);

  if (telemetryHistory.length < 2) return;

  // Determine value bounds
  const minVal = 0;
  const maxVal = 100;

  // Draw subtle horizontal grid lines
  ctx.strokeStyle = "rgba(255, 255, 255, 0.06)";
  ctx.lineWidth = 1;
  ctx.fillStyle = "rgba(255, 255, 255, 0.35)";
  ctx.font = "11px 'JetBrains Mono', monospace";
  ctx.textAlign = "right";

  const gridSteps = 4;
  for (let i = 0; i <= gridSteps; i++) {
    const val = minVal + (maxVal - minVal) * (i / gridSteps);
    const y = padding.top + chartH - (i / gridSteps) * chartH;
    ctx.beginPath();
    ctx.moveTo(padding.left, y);
    ctx.lineTo(padding.left + chartW, y);
    ctx.stroke();
    ctx.fillText(`${Math.round(val)}`, padding.left - 8, y + 4);
  }

  // Draw X axis time labels
  ctx.textAlign = "center";
  const stepX = chartW / (telemetryHistory.length - 1);
  telemetryHistory.forEach((pt, idx) => {
    const x = padding.left + idx * stepX;
    if (idx % 2 === 0 || idx === telemetryHistory.length - 1) {
      ctx.fillText(pt.time, x, h - 10);
    }
  });

  // Helper: Plot continuous smooth line
  function plotLine(key, strokeColor, fillColor) {
    ctx.beginPath();
    const coords = telemetryHistory.map((pt, idx) => {
      const x = padding.left + idx * stepX;
      const y = padding.top + chartH - ((pt[key] - minVal) / (maxVal - minVal)) * chartH;
      return { x, y };
    });

    coords.forEach((coord, idx) => {
      if (idx === 0) ctx.moveTo(coord.x, coord.y);
      else ctx.lineTo(coord.x, coord.y);
    });

    ctx.strokeStyle = strokeColor;
    ctx.lineWidth = 2.5;
    ctx.stroke();

    // Area fill
    ctx.lineTo(padding.left + chartW, padding.top + chartH);
    ctx.lineTo(padding.left, padding.top + chartH);
    ctx.closePath();
    ctx.fillStyle = fillColor;
    ctx.fill();

    // Plot highlight dots
    coords.forEach((c) => {
      ctx.beginPath();
      ctx.arc(c.x, c.y, 3.5, 0, Math.PI * 2);
      ctx.fillStyle = strokeColor;
      ctx.fill();
    });
  }

  // Draw Humidity (Cyan)
  const gradHum = ctx.createLinearGradient(0, padding.top, 0, padding.top + chartH);
  gradHum.addColorStop(0, "rgba(6, 182, 212, 0.25)");
  gradHum.addColorStop(1, "rgba(6, 182, 212, 0.0)");
  plotLine("humidity", "#06b6d4", gradHum);

  // Draw Temperature (Orange/Emerald)
  const gradTemp = ctx.createLinearGradient(0, padding.top, 0, padding.top + chartH);
  gradTemp.addColorStop(0, "rgba(249, 115, 22, 0.25)");
  gradTemp.addColorStop(1, "rgba(249, 115, 22, 0.0)");
  plotLine("temp", "#f97316", gradTemp);

  // Legend
  ctx.textAlign = "left";
  ctx.fillStyle = "#f97316";
  ctx.fillText("● Temp (°C)", padding.left + 10, padding.top - 5);
  ctx.fillStyle = "#06b6d4";
  ctx.fillText("● Humidity (%)", padding.left + 95, padding.top - 5);
}

window.addEventListener("resize", renderTelemetryChart);
renderTelemetryChart();

// ------------------------------------------------------------------
// 6. ESP32 Simulator / Testing Controls
// Pushes simulated telemetry directly to sensorData/latest
// to demonstrate live real-time synchronization.
// ------------------------------------------------------------------
async function sendSimulatedTelemetry(payload, btn) {
  try {
    if (btn) btn.disabled = true;
    const latestDocRef = doc(db, "sensorData", "latest");
    await setDoc(latestDocRef, {
      ...payload,
      timestamp: serverTimestamp(),
      updatedAt: new Date().toISOString(),
      simulated: true,
      deviceId: "ESP32-SIM-01"
    });
    console.log("Simulated telemetry published to Firestore:", payload);
  } catch (err) {
    console.error("Failed to publish simulated telemetry:", err);
    alert("Simulation write error: " + err.message);
  } finally {
    if (btn) btn.disabled = false;
  }
}

if (simNormalBtn) {
  simNormalBtn.addEventListener("click", () => {
    // Generate slight realistic variations
    const randomTemp = (23.5 + Math.random() * 2.0).toFixed(1);
    const randomHum = (50 + Math.random() * 8.0).toFixed(1);
    sendSimulatedTelemetry({
      temperature: parseFloat(randomTemp),
      humidity: parseFloat(randomHum),
      airQuality: "Good (38 AQI)",
      smokeLevel: "Safe (110 PPM)"
    }, simNormalBtn);
  });
}

if (simWarningBtn) {
  simWarningBtn.addEventListener("click", () => {
    sendSimulatedTelemetry({
      temperature: 37.8,
      humidity: 82.5,
      airQuality: "Poor (192 AQI)",
      smokeLevel: "Alert (650 PPM)"
    }, simWarningBtn);
  });
}

// ------------------------------------------------------------------
// 7. Sidebar Interactions & Mobile Toggle
// ------------------------------------------------------------------
if (mobileNavToggle && sidebarEl) {
  mobileNavToggle.addEventListener("click", () => {
    sidebarEl.classList.toggle("open");
  });
}

const sidebarLinks = document.querySelectorAll(".sidebar-link");
sidebarLinks.forEach((link) => {
  link.addEventListener("click", (e) => {
    sidebarLinks.forEach((l) => l.classList.remove("active"));
    link.classList.add("active");
  });
});
