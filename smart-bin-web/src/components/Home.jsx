import React, { useEffect, useState } from 'react';
import { ref, onValue, update } from 'firebase/database';
import { database } from '../firebase';
import Navbar from './Navbar';
import BinCard from './BinCard';

const Home = () => {
  const [bins, setBins] = useState({});

  useEffect(() => {
    const binsRef = ref(database, '/');
    onValue(binsRef, (snapshot) => {
      const data = snapshot.val();
      setBins(data);
    });
  }, []);

  const onUpdateBin = async (binId, updatedData) => {
    try {
      const binRef = ref(database, `/${binId}`);
      await update(binRef, updatedData);
      console.log(`Bin ${binId} updated successfully`);
    } catch (error) {
      console.error(`Error updating bin ${binId}:`, error);
      throw error; // Re-throw the error so BinCard can handle it
    }
  };

  return (
    <div>
      <Navbar />
      <div style={{ display: 'flex', flexWrap: 'wrap' }}>
        {Object.entries(bins).map(([binId, binData]) => (
          <BinCard key={binId} binId={binId} binData={binData} onUpdateBin={onUpdateBin} />
        ))}
      </div>
    </div>
  );
};

export default Home;
