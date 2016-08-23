Test by Tanmoy
20160821
Mac arch

nummemes  = 10, numlexes  = 15, numagents = 40
mutrate = 1, penalty = 100
inner = 100, outer = 1000
Random number generator seeded with 123 

main
    main.c++: $Id: 4a4f075c31e2919cfeb8397c643e1ad4415786ae $
    main.h++: $Id: 1d9439e7ebccf824a71ae333abf29c308ebbe766 $
    selfiterator.h++: $Id: afa1b371ef4b897bca83eeaa4325f0545bf706fe $
    language.h++: $Id: 0d670c2ffc6ef143fc9014f605def80659ede712 $
    counts.h++: $Id: d508d6f6e3b919105263f15888e050853c2d29fa $
    agent.h++: $Id: 54ace7ea8fd8fb8b3f3d6600ec42a376e90cf359 $
    lex.h++: $Id: 1f721bb364c5a607665e412a5d9b7b61ceb0e98d $
    meme.h++: $Id: 2784b0de8b589f76554adc1bbfd1fb3322f6c300 $
    enum.h++: $Id: 0e2d7da0ffba72dbc04251d09447baabdd9b47c9 $
    network.h++: $Id: eac51fb06e968170316f54b584aa2b2a1e168f11 $
    probvector.h++: $Id: 2d35c213256996921aed45e810891f3615620bd3 $
    myutil.h++: $Id: 2edf6d8246de5783d36f7f09a80bf41f9f9b83e1 $
    enumvector.h++: $Id: db63042aff05dc2564392912b3cdcc66c251e032 $

Command:
(what main; echo 10 15 40 1 100 100 1000 123 | ./main) > check.txt 2>&1

Note: in order to have git update the what Id checksums, we need to
      commit AND check out to give it a chance to update th checksum.

