import React, { useEffect, useState } from 'react';
import { ref, onValue } from 'firebase/database';
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

  return (
    <div>
      <Navbar />
      <div style={{ display: 'flex', flexWrap: 'wrap' }}>
        {Object.entries(bins).map(([binId, binData]) => (
          <BinCard key={binId} binId={binId} binData={binData} />
        ))}
      </div>
    </div>
  );
};

export default Home;
