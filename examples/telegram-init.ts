/**
 * Telegram Mini App Initialization Example
 * 
 * This file demonstrates how to properly initialize a Telegram Mini App
 * and get user information and shop token from deep links.
 */

import WebApp from '@twa-dev/sdk';

// Initialize Telegram WebApp
export function initTelegramApp() {
  console.log('Initializing Telegram Mini App...');

  // Tell Telegram the app is ready
  WebApp.ready();

  // Expand to full height
  WebApp.expand();

  // Disable vertical swipes (optional)
  WebApp.disableVerticalSwipes();

  console.log('✅ Telegram Mini App initialized');
}

// Get user information
export function getUserInfo() {
  const user = WebApp.initDataUnsafe.user;

  if (!user) {
    console.warn('⚠️  User info not available. Make sure you are running inside Telegram.');
    return null;
  }

  return {
    id: user.id,
    firstName: user.first_name,
    lastName: user.last_name,
    username: user.username,
    languageCode: user.language_code,
    isPremium: user.is_premium,
  };
}

// Get shop token from deep link
export function getShopToken(): string {
  const shopToken = WebApp.initDataUnsafe.start_param || 'demo';
  
  console.log('Shop token from deep link:', shopToken);
  
  // Deep link format: https://t.me/yourbot?start=shop_token_here
  // This will give start_param = 'shop_token_here'
  
  return shopToken;
}

// Setup theme colors
export function applyTelegramTheme() {
  const theme = WebApp.themeParams;

  if (theme.bg_color) {
    document.documentElement.style.setProperty('--tg-theme-bg-color', theme.bg_color);
  }
  if (theme.text_color) {
    document.documentElement.style.setProperty('--tg-theme-text-color', theme.text_color);
  }
  if (theme.hint_color) {
    document.documentElement.style.setProperty('--tg-theme-hint-color', theme.hint_color);
  }
  if (theme.link_color) {
    document.documentElement.style.setProperty('--tg-theme-link-color', theme.link_color);
  }
  if (theme.button_color) {
    document.documentElement.style.setProperty('--tg-theme-button-color', theme.button_color);
  }
  if (theme.button_text_color) {
    document.documentElement.style.setProperty('--tg-theme-button-text-color', theme.button_text_color);
  }

  console.log('✅ Theme colors applied');
}

// Setup back button
export function setupBackButton(onBack: () => void) {
  WebApp.BackButton.onClick(onBack);
  WebApp.BackButton.show();

  return () => {
    WebApp.BackButton.offClick(onBack);
    WebApp.BackButton.hide();
  };
}

// Setup main button
export function setupMainButton(text: string, onClick: () => void) {
  WebApp.MainButton.setText(text);
  WebApp.MainButton.onClick(onClick);
  WebApp.MainButton.show();

  return {
    hide: () => WebApp.MainButton.hide(),
    disable: () => WebApp.MainButton.disable(),
    enable: () => WebApp.MainButton.enable(),
    showProgress: () => WebApp.MainButton.showProgress(),
    hideProgress: () => WebApp.MainButton.hideProgress(),
    setText: (newText: string) => WebApp.MainButton.setText(newText),
    cleanup: () => {
      WebApp.MainButton.offClick(onClick);
      WebApp.MainButton.hide();
    },
  };
}

// Show alert
export function showAlert(message: string, callback?: () => void) {
  WebApp.showAlert(message, callback);
}

// Show confirmation
export function showConfirm(message: string): Promise<boolean> {
  return new Promise((resolve) => {
    WebApp.showConfirm(message, (confirmed) => {
      resolve(confirmed);
    });
  });
}

// Complete initialization example
export function completeSetup() {
  console.log('🚀 Setting up Telegram Mini App...\n');

  // 1. Initialize
  initTelegramApp();

  // 2. Get user info
  const user = getUserInfo();
  if (user) {
    console.log('👤 User:', user);
  }

  // 3. Get shop token
  const shopToken = getShopToken();
  console.log('🏪 Shop Token:', shopToken);

  // 4. Apply theme
  applyTelegramTheme();

  // 5. Log device info
  console.log('📱 Platform:', WebApp.platform);
  console.log('🎨 Color Scheme:', WebApp.colorScheme);
  console.log('📏 Viewport Height:', WebApp.viewportHeight);
  console.log('🔒 Viewport Stable:', WebApp.viewportStableHeight);

  console.log('\n✅ Setup complete!');

  return {
    user,
    shopToken,
    platform: WebApp.platform,
    colorScheme: WebApp.colorScheme,
  };
}

// Example usage in a React component
export function ExampleUsage() {
  /*
  import { useEffect } from 'react';
  import { completeSetup, setupBackButton, setupMainButton, showConfirm } from './telegram-init';

  function MyApp() {
    useEffect(() => {
      // Initialize Telegram app
      const { user, shopToken } = completeSetup();

      // Setup back button
      const cleanupBack = setupBackButton(() => {
        window.history.back();
      });

      // Setup main button
      const mainButton = setupMainButton('Checkout', async () => {
        const confirmed = await showConfirm('Proceed to checkout?');
        if (confirmed) {
          // Handle checkout
        }
      });

      // Cleanup on unmount
      return () => {
        cleanupBack();
        mainButton.cleanup();
      };
    }, []);

    return (
      <div>
        <h1>My Telegram Mini App</h1>
      </div>
    );
  }
  */
}

// Export everything
export default {
  init: initTelegramApp,
  getUserInfo,
  getShopToken,
  applyTheme: applyTelegramTheme,
  setupBackButton,
  setupMainButton,
  showAlert,
  showConfirm,
  completeSetup,
};
