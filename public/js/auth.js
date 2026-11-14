/* ==========================================================
   SMART ENVIRONMENT MONITOR - AUTHENTICATION MODULE (auth.js)
   Handles Signup, Login, Logout, Auth state detection,
   and Firestore User Profile creation using Firebase Modular SDK.
   ========================================================== */

import { auth, db } from "./firebase.js";
import {
  createUserWithEmailAndPassword,
  signInWithEmailAndPassword,
  signOut,
  onAuthStateChanged
} from "https://www.gstatic.com/firebasejs/12.1.0/firebase-auth.js";

import {
  doc,
  setDoc,
  serverTimestamp
} from "https://www.gstatic.com/firebasejs/12.1.0/firebase-firestore.js";

// Helper: Translate Firebase error codes into clear, user-friendly messages
function getFriendlyErrorMessage(error) {
  const code = error.code || "";
  switch (code) {
    case "auth/email-already-in-use":
      return "This email address is already registered. Please sign in instead.";
    case "auth/invalid-email":
      return "Please enter a valid email address.";
    case "auth/operation-not-allowed":
      return "Email/Password sign-in is not enabled in Firebase Console.";
    case "auth/weak-password":
      return "Password is too weak. Please use at least 6 characters.";
    case "auth/user-disabled":
      return "This user account has been disabled.";
    case "auth/user-not-found":
      return "No account found with this email address.";
    case "auth/wrong-password":
      return "Incorrect password. Please double-check and try again.";
    case "auth/invalid-credential":
      return "Invalid email or password. Please check your credentials and try again.";
    case "auth/too-many-requests":
      return "Too many unsuccessful attempts. Access temporarily blocked. Try again later.";
    case "auth/network-request-failed":
      return "Network error. Please verify your internet connection.";
    default:
      return error.message || "An unexpected authentication error occurred.";
  }
}

// Helper: Show error banner
function showError(message) {
  const errorEl = document.getElementById("authError");
  if (errorEl) {
    errorEl.textContent = message;
    errorEl.style.display = "block";
  } else {
    alert(message);
  }
}

// Helper: Hide error banner
function clearError() {
  const errorEl = document.getElementById("authError");
  if (errorEl) {
    errorEl.textContent = "";
    errorEl.style.display = "none";
  }
}

// Helper: Toggle submit button loading state
function setButtonLoading(btn, isLoading, defaultText, loadingText) {
  if (!btn) return;
  if (isLoading) {
    btn.disabled = true;
    btn.dataset.originalText = btn.textContent;
    btn.textContent = loadingText;
  } else {
    btn.disabled = false;
    btn.textContent = defaultText || btn.dataset.originalText || "Submit";
  }
}

// ------------------------------------------------------------------
// Auth State Observer:
// If an already logged-in user navigates to login.html or signup.html,
// redirect them directly to dashboard.html.
// ------------------------------------------------------------------
onAuthStateChanged(auth, (user) => {
  const currentPath = window.location.pathname.toLowerCase();
  const isAuthPage = currentPath.endsWith("login.html") || currentPath.endsWith("signup.html");
  
  if (user && isAuthPage) {
    console.log("Active session detected, redirecting to dashboard:", user.email);
    window.location.replace("dashboard.html");
  }
});

// ------------------------------------------------------------------
// Signup Handler (signup.html)
// ------------------------------------------------------------------
const signupForm = document.getElementById("signupForm");
if (signupForm) {
  signupForm.addEventListener("submit", async (e) => {
    e.preventDefault();
    clearError();

    const nameInput = document.getElementById("signupName");
    const emailInput = document.getElementById("signupEmail");
    const passwordInput = document.getElementById("signupPassword");
    const confirmPasswordInput = document.getElementById("signupConfirmPassword");
    const submitBtn = signupForm.querySelector("button[type='submit']");

    const name = nameInput ? nameInput.value.trim() : "";
    const email = emailInput ? emailInput.value.trim() : "";
    const password = passwordInput ? passwordInput.value : "";
    const confirmPassword = confirmPasswordInput ? confirmPasswordInput.value : "";

    // Validation checks
    if (!email || !password) {
      showError("Please fill in all required fields.");
      return;
    }

    if (password.length < 6) {
      showError("Password must be at least 6 characters long.");
      return;
    }

    if (confirmPasswordInput && password !== confirmPassword) {
      showError("Passwords do not match. Please re-enter.");
      return;
    }

    try {
      setButtonLoading(submitBtn, true, "Create Account", "Creating account...");

      // 1. Create User in Firebase Auth
      const userCredential = await createUserWithEmailAndPassword(auth, email, password);
      const user = userCredential.user;
      console.log("Firebase user created successfully:", user.uid);

      // 2. Create User Profile document in Firestore: users/{user.uid}
      try {
        await setDoc(doc(db, "users", user.uid), {
          name: name || email.split("@")[0],
          email: email,
          createdAt: serverTimestamp()
        });
        console.log("User document recorded in Firestore");
      } catch (firestoreErr) {
        // Non-blocking warning: Auth succeeded, profile write may require rule permission
        console.warn("Could not save initial Firestore user doc:", firestoreErr);
      }

      // 3. Redirect to dashboard
      window.location.replace("dashboard.html");
    } catch (err) {
      console.error("Signup error:", err);
      showError(getFriendlyErrorMessage(err));
      setButtonLoading(submitBtn, false, "Create Account", "");
    }
  });
}

// ------------------------------------------------------------------
// Login Handler (login.html)
// ------------------------------------------------------------------
const loginForm = document.getElementById("loginForm");
if (loginForm) {
  loginForm.addEventListener("submit", async (e) => {
    e.preventDefault();
    clearError();

    const emailInput = document.getElementById("loginEmail");
    const passwordInput = document.getElementById("loginPassword");
    const submitBtn = loginForm.querySelector("button[type='submit']");

    const email = emailInput ? emailInput.value.trim() : "";
    const password = passwordInput ? passwordInput.value : "";

    if (!email || !password) {
      showError("Please enter both your email address and password.");
      return;
    }

    try {
      setButtonLoading(submitBtn, true, "Login", "Signing in...");

      const userCredential = await signInWithEmailAndPassword(auth, email, password);
      console.log("Logged in successfully as:", userCredential.user.email);

      // Redirect to dashboard
      window.location.replace("dashboard.html");
    } catch (err) {
      console.error("Login error:", err);
      showError(getFriendlyErrorMessage(err));
      setButtonLoading(submitBtn, false, "Login", "");
    }
  });
}

// ------------------------------------------------------------------
// Global Logout Handler (if present on any auth-loaded page)
// ------------------------------------------------------------------
const logoutButton = document.getElementById("logoutButton");
if (logoutButton) {
  logoutButton.addEventListener("click", async () => {
    try {
      await signOut(auth);
      console.log("User signed out");
      window.location.replace("login.html");
    } catch (err) {
      console.error("Sign out error:", err);
      alert("Error signing out. Please try again.");
    }
  });
}
