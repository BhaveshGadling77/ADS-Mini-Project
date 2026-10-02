import { useState } from "react";
import { FlatList, Image, Pressable, RefreshControl, ScrollView, StyleSheet, Text, TextInput, View } from "react-native";


type DataType = {
  id: string;
  title: string;
};

const DATA = [      //data for flatlist
  { id: '1', title: 'Learn FlatList' },
  { id: '2', title: 'Master State' },
  { id: '3', title: 'Build an App' },
  { id: '4', title: 'Learn FlatList' },
  { id: '5', title: 'Master State' },
  { id: '6', title: 'Build an App' },
  { id: '7', title: 'Learn FlatList' },
  { id: '8', title: 'Master State' },
  { id: '9', title: 'Build an App' },
];


export default function Index() {
  const [text, setText] = useState("");
  const [text2, setText2] = useState("");
  const [Status, status] = useState("");

  const [pressedCount, setPressedCount] = useState(0);
  const[btnstats, setbtnstats] = useState("");

  const [refreshing, setRefreshing] = useState(false);
  const onRefresh = () => {  // This function runs when the user pulls down from the top
    setRefreshing(true);
    setTimeout(() => {
      setRefreshing(false);
    }, 1000);   //Simulate a network request taking 1 seconds
  };

  // The function that tells FlatList how to draw a single item
  // Note the { item } syntax – you have to destructure it this way!
  const renderItem = ({ item }: { item: DataType }) => (
    <View style={abc.flatcontainer}>
      <Text style={abc.flatbox}>{item.title}</Text>
    </View>
  );

return (
    <View style={[styles.retainer, abc.one]}>
      <Text>Edit src/app/index.tsx to edit this screen.</Text>
      <View style={abc.two}>
        <Text>nested view</Text>
        
        {/* _____________________________________________________________________________________________________________ */}
        <Image source={require("../../assets/images/icon.png")} style={abc.logo}/>
        <Image source={{uri : "https://media.architecturaldigest.com/photos/66758a12539a07cf706eefaa/16:9/w_1920,c_limit/EMBARGO-BUGATTI-World-Premiere-Presskit-Images-26.jpg"}} style={abc.car}/>

        {/* _____________________________________________________________________________________________________________ */}      
        <TextInput
          style={abc.inpt}   
          placeholder="Type here..."
          onChangeText={setText}                                  
          value={text}
        />
        <Text style={abc.outpt}>Hello, {text}!</Text>
        
        <TextInput
          style={abc.inpt}
          placeholder="Type here..."
          onEndEditing={(e) => setText2(e.nativeEvent.text)}      
          defaultValue={text2}
        />
        <Text style={abc.outpt}>Hello, {text2}!</Text>
        
        <TextInput
          style={abc.inpt}
          placeholder="Type here..."
          onBlur={() => status("User tapped away from input.")}
          onFocus={() => status("Input is currently focused!")}
          onSubmitEditing={(e) => status(e.nativeEvent.text)}
          defaultValue={Status}
        />
        <Text style={abc.outpt}>Status: {Status}!</Text>
        
        {/* _____________________________________________________________________________________________________________ */}      
        <Pressable
          style={({ pressed }) => [{ backgroundColor: pressed ? '#ddd' : '#bbb' }, abc.button]}
          onPress={() => setPressedCount(pressedCount + 1)}     
          onLongPress={() => setbtnstats("You held down the button!")}          
          delayLongPress={1000}         
          onPressIn={() => setbtnstats("You pressed down the button!")}        
          onPressOut={() => setbtnstats("You released the button!")}
          hitSlop={{ top: 20, bottom: 50, left: 20, right: 20 }}
        >
          <Text>Press me !</Text>
        </Pressable>
        <Text style={abc.outpt}>btnstats : {btnstats}! | Pressed {pressedCount} times</Text>
        
        {/* _____________________________________________________________________________________________________________ */}
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
          <View style={abc.box}><Text>Item 4</Text></View>
          <View style={abc.box}><Text>Item 5</Text></View>
          <View style={abc.box}><Text>Item 6</Text></View>
        </ScrollView>
        
        {/* _____________________________________________________________________________________________________________ */}
        <FlatList
          data={DATA}
          renderItem={renderItem}
          keyExtractor={(item) => item.id}
          ListHeaderComponent={<Text style={abc.flatbox}>Tasks</Text>}
          ListEmptyComponent={<Text style={abc.flatbox}>Your cart is empty.</Text>}
          ItemSeparatorComponent={() => <View style={{height: 2, backgroundColor: '#ccc'}} />}
          onEndReached={() => console.log('Fetch more items!')}
          onEndReachedThreshold={0.5}
        />
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    backgroundColor: "#fff",
  },
  retainer: {
    backgroundColor: "#700",
  },
});

const abc = StyleSheet.create({
  one: {
    backgroundColor: "#475",
    flex: 1,
  },
  two: {
    backgroundColor: "#f00",
  },
  logo: {
    height: 100,
    width: 100,
    margin: 10,
  },
  car: {
    height: 100,
    width: 100,
  },
  inpt: {
    backgroundColor: "#ff0",
  },
  outpt: {
    fontSize: 18,
    fontWeight: "bold",
  },
  button: {
    alignItems: 'center',
    paddingVertical: 12,
    borderRadius: 40,
  },
  container: {
    backgroundColor: '#71de18',
    padding: 5,
  },
  box: {
    width: 200, // Large width to push content off-screen
    justifyContent: 'center',
    alignItems: 'center',
    borderRadius: 10,
  },
  flatcontainer: {
    backgroundColor: '#818e99',
  },
  flatbox: {
    height: 100, // Large height to push content off-screen
    margin: 10,
    borderRadius: 10,
  },
});