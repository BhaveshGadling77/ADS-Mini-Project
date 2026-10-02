import { Tabs } from 'expo-router';
import { Image } from 'react-native';

export default function RootLayout() {
  return (
    <Tabs>
      <Tabs.Screen
        name="index"
        options={{
          title: 'Home',
          tabBarIcon: ({ focused, color, size }) => (
            <Image 
              source={require("../../assets/images/android-icon-foreground.png")} 
              style={{ width: size || 24, height: size || 24}}
            />
          ), 
        }}
      />
      <Tabs.Screen
        name="profile"
        options={{
          title: 'Profile',
          tabBarIcon: ({ focused, color, size }) => (
            <Image 
              source={require("../../assets/images/react-logo.png")} 
              style={{ width: size || 24, height: size || 24}}
            />
          ), 
        }}
      />
      <Tabs.Screen 
        name="temp1" 
        options={{ 
          title: "tmp1", 
          tabBarLabel: "tp1", 
          tabBarIcon: ({ focused, color, size }) => (
            <Image 
              source={require("../../assets/images/icon.png")} 
              style={{ width: size || 24, height: size || 24}}
            />
          ), 
          headerShown: true}}
      />
    </Tabs>
  );
}