Charset
=======

.. highlight:: apache

Synopsis
--------
To enable special characters with accents or non latin characters, you have to define this tag.

Options
-------

========== ========= ============= ============= =======
Name       Default   Values        Since version Context
========== ========= ============= ============= =======
Charset    null      United States 0.8           Default
\                    ISO-8859-1
\                    ISO-8859-2
\                    ISO-8859-3
\                    ISO-8859-4
\                    ISO-8859-5
\                    ISO-8859-6
\                    ISO-8859-7
\                    ISO-8859-9
\                    ISO-8859-13
\                    ISO-8859-15
\                    KOI8-R
\                    KOI8-U/RU
\                    UTF8
========== ========= ============= ============= =======

Examples
--------
Here we're going to change the charset to a common one::

    <Default>
        Charset "UTF8"
    </Default>

Here, we're using UTF8 charset.
