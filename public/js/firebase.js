import { initializeApp } from "https://www.gstatic.com/firebasejs/12.1.0/firebase-app.js";
import { getAuth } from "https://www.gstatic.com/firebasejs/12.1.0/firebase-auth.js";
import { getFirestore } from "https://www.gstatic.com/firebasejs/12.1.0/firebase-firestore.js";

const firebaseConfig = {
    apiKey: "AIzaSyDfTBoWyVvxItwMqBDAFZdOi_kvPa7agBY",
    authDomain: "smart-environment-monito-3c43f.firebaseapp.com",
    projectId: "smart-environment-monito-3c43f",
    storageBucket: "smart-environment-monito-3c43f.firebasestorage.app",
    messagingSenderId: "301278903288",
    appId: "1:301278903288:web:7fec37baef0d22ba2d740d"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);

// Firebase Authentication
const auth = getAuth(app);

// Firestore Database
const db = getFirestore(app);

export { app, auth, db };