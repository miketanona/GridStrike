# GridStrike – Design Notes

---

## 🔍 Search Algorithm

This is what initially grabbed my interest and became the thrust of my application. I was particularly interested in evaluating the search engines with large numbers of variable start and end points, and charting them as a function of the varying Manhattan distances:  
[https://en.wikipedia.org/wiki/Taxicab_geometry](https://en.wikipedia.org/wiki/Taxicab_geometry)

I was hoping to see some obvious patterns—such as one search engine being better (or faster) at evaluating short distances, and others better at long, more complex travel paths. This could be valuable to someone in the field; for example, using certain strategies when an enemy is in close proximity and another when they are farther away. It could also be useful in code, evaluating the sample size and using a switch statement to dynamically select different search (or render or other) components in an uncertain environment.

I've always felt that many of the key components—search engines, graphics rendering, database queries and updates—become buried in applications and turn into sacred cows that no one wants to touch. However, if they can be pulled out into components in a way that makes them easier to evaluate, with many and varied parameters—ideally with a comprehensible UI component—they can be put into a simulation mode for testing and evaluation.

In my opinion, you don't need to over-refactor or componentize every little thing in your app, but for certain components, it really does add a lot of value. I fully intended to extract the search algorithms into a `SearchEngine` component, but didn’t get the time to do it without risking last-minute instability.

---

## 🖼 UI

I went with OpenGL—mainly for the wow effect, which I remembered from previous projects—and because it didn't require subsequent developers to download any third-party libraries (at least that was my belief at the time; that statement should never be made unequivocally).  
In hindsight, probably a mistake.

It doesn’t come with any built-in methods for adding things like labels, axes, or legends, so you end up rolling your own. Bad idea on projects with short deliverables. You end up wasting time writing hacks to make the overall UI presentable, as you don’t want something like a chart legend overwriting data points and ruining the whole effect.

I use Matplotlib for many of my AWS web-hosted apps, and while the visuals aren’t stunning, the axes are almost always drawn and placed correctly, and most of the accessories have reasonable default positions. So if you don’t have a lot of extra time, you don’t need to adjust much.

Naturally, this was the first element I thought to componentize—so I created the `GridChartRenderer` class. It would be nice to see this run side-by-side with, say, Qt Charts, GDI+, or Direct2D. Better yet: being able to swap out the UI with other rendering engines or run them in parallel for comparison.

---

## 🧵 Multi-Threading

I decided to make this multithreaded. I envisioned the search image/charting running for a significant amount of time, with maybe 3 or 4 search engines executing concurrently before a pattern emerged and the user could decide to stop it.

As it turned out, it ran much faster than expected, and I only had two search methods—so it wasn’t really necessary. But it’s there for the future and didn’t add much overhead.

---

## 🪞 Presentation

I never could figure out if this was meant for a single search-and-draw (the basic assignment) or for running simulations with random start and end points. I figure you should always keep the original assignment in mind, so it’s a bit of a hodgepodge. Originally, I planned on hiding the charting window and auto-resizing, but in some environments that’s frowned upon—and given the time constraints, I left it as is.

---

## 📣 Feedback

Discounting my initial reaction 🙂, I think this is a great idea for evaluating technical talent. Certainly better than whiteboarding code on the fly—which ends up being a “who can do simple tasks under short-window pressure” test. The pressure software engineers experience is usually more like: “I need this in a day or two,” or—when critical—“by this afternoon.”

Folks who can handle that well may struggle with grade-school chalkboard challenges, and then you might end up filtering out some of the best candidates. Besides, when it’s over, you may actually pick up a new skill or two—or have something you can reuse and be proud of.
