import { useState } from "react";
import { RefreshControl, ScrollView, StyleSheet, Text, View } from "react-native";

export default function Index() {

      const [refreshing, setRefreshing] = useState(false);
      const onRefresh = () => {  // This function runs when the user pulls down from the top
        setRefreshing(true);
        setTimeout(() => {
          setRefreshing(false);
        }, 1000);   //Simulate a network request taking 1 seconds
      };

    return (
    <View>
        <Text style={abc.text}>Profile</Text>
                <ScrollView style={abc.container}
                  horizontal={true}
                  contentContainerStyle={{ alignItems: 'center', justifyContent: 'center', paddingHorizontal: 20 }}
                  showsHorizontalScrollIndicator={false}
                  keyboardShouldPersistTaps="handled"
                  pagingEnabled={true}
                  refreshControl={<RefreshControl refreshing={refreshing} onRefresh={onRefresh} />}
                >
              
                  {/* These boxes take up a lot of height, forcing the screen to scroll horizontally */}
                  <View style={abc.box}><Text>Item 1</Text></View>
                  <View style={abc.box}><Text>Item 2</Text></View>
                  <View style={abc.box}><Text>Item 3</Text></View>
                </ScrollView>
    </View>
    );
}

const abc = StyleSheet.create({
    text:{
        padding: 30,
    },
  container: {
    backgroundColor: '#71de18',
    padding: 5,
  },
  box: {
    width: 400, // Large width to push content off-screen
    height: 200, // Large height to push content off-screen
    justifyContent: 'center',
    alignItems: 'center',
    borderRadius: 10,
  },
});